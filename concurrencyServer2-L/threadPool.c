#include "threadPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

/* 默认最大最小值 */
#define DEFAULT_MIN_THREAD_NUM 0
#define DEFAULT_MAX_THREAD_NUM 10
#define DEFAULT_QUEUE_MAX_SIZE 100
#define TIME_INTERVAL 5
#define DEFAULT_EXPANSION_SIZE 3
#define DEFAULT_REDUCTION_SIZE 3

/* 状态码 */
enum STATUS_CODE
{
    SUCCESS = 0,
    NULL_PTR,
    MALLOC_ERROR,
    ACCESS_INVAILD,
    THREAD_CREATE_ERROR,
    THREAD_JOIN_ERROR,
    THREAD_EXIT_ERROR,
    THREAD_MUTEX_ERROR,
    THREAD_COND_ERROR,
    MUTEX_DESTROY_ERROR,
    UNKNOWN_ERROR,
    
};

/* 静态函数前置声明 */
static void *threadHander(void *arg);
static void *mangerHander(void *arg);

/* 线程退出清除资源 */
static int threadExitClrResources(threadPool_t *pool)
{
    for(int idx = 0; idx < pool->maxSize; idx++)
    {
        if(pool->threadID[idx] == pthread_self())
        {
            pool->threadID[idx] = 0;
            break;
        }
    }
    // pthread_exit(NULL);
    return SUCCESS;
}

/* 线程函数 */
static void *threadHander(void *arg)
{
    threadPool_t *pool = (threadPool_t *)arg;
    while(1)
    {
        /* 加锁 */
        pthread_mutex_lock(&pool->mutex);
        while(pool->queueSize == 0 && pool->shutdown != 1)
        {
            /* 等待一个条件变量被唤醒 */
            pthread_cond_wait(&pool->notEmpty, &pool->mutex);

        }
        if(pool->exitSize > 0)
        {
            pool->exitSize--;
            if(pool->aliveSize > pool->minSize)
            {
                
                /* 解锁 */
                pthread_mutex_unlock(&pool->mutex);
                /* 释放资源 */
                threadExitClrResources(pool);
                /* 退出 */
                pthread_exit(NULL);
            }
        }
        /* 出队 */
        task_t task = pool->taskQueue[pool->queueFront]; 
        pool->queueFront = (pool->queueFront + 1) % pool->queueCapacity;
        /* 队列大小-1 */
        pool->queueSize--;
        /* 解锁 */
        pthread_mutex_unlock(&pool->mutex);
        /* 唤醒生产者 */
        pthread_cond_signal(&pool->notFull);

        /* 为了提升性能，在创建一把只维护busySize的锁 */
        pthread_mutex_lock(&pool->busyMutex);
        /* 增加busySize */
        pool->busySize++;
        /* 解锁 */
        pthread_mutex_unlock(&pool->busyMutex);

        /* 执行任务 */
        task.function(task.arg);
        /* 释放任务 */
        free(task.arg);
        task.arg = NULL;
        free(task.function);
        task.function = NULL;

        pthread_mutex_lock(&pool->busyMutex);
        /* 减少busySize */
        pool->busySize--;
        /* 解锁 */
        pthread_mutex_unlock(&pool->busyMutex);

        

    }
}

/* 管理员线程 */
static void *mangerHander(void *arg)
{
    threadPool_t *pool = (threadPool_t *)arg;
    while(!pool->shutdown)
    {
        sleep(TIME_INTERVAL);

        /* 获取线程池信息 */
        pthread_mutex_lock(&pool->mutex);
        int taskNum = pool->queueSize;
        int aliveSize = pool->aliveSize;
        pthread_mutex_unlock(&pool->mutex);

        pthread_mutex_lock(&pool->busyMutex);
        int busySize = pool->busySize;
        pthread_mutex_unlock(&pool->busyMutex);

        /* 扩容: 任务数>存活线程数 && 存活线程数<最大线程数  */
        if (taskNum > aliveSize && aliveSize < pool->maxSize)
        {
            /* 一次扩大3个 */
            int count = 0;
            int ret = 0;
            pthread_mutex_lock(&pool->mutex);
            for (int idx = 0; count < DEFAULT_EXPANSION_SIZE && idx < pool->maxSize; idx++)
            {
                if (pool->threadID[idx] == 0)
                {
                    ret = pthread_create(&pool->threadID[idx], NULL, threadHander, pool);
                    if (ret != 0)
                    {
                        perror("pthread_create error");
                        break; /* todo */
                    }
                    count++;
                    pool->aliveSize++;
                }
            }
            pthread_mutex_unlock(&pool->mutex);
        }

        
        /* 缩容: 忙线程数*2>存活线程数 && 存活线程数>最小线程数 */
        if ((busySize << 1) > aliveSize && aliveSize > pool->minSize)
        {
            pthread_mutex_lock(&pool->mutex);

            /* 设置退出线程的数量 */
            pool->exitSize = DEFAULT_REDUCTION_SIZE;
            /* 唤醒所有等待线程 */
            pthread_cond_broadcast(&pool->notEmpty);

            pthread_mutex_unlock(&pool->mutex);
        }
    }
    /* 管理线程退出 */
    pthread_exit(NULL);
}

/* 线程池初始化 */
int threadPoolInit(threadPool_t *pool, int minSize, int maxSize, int queueCapacity)
{
    if(pool == NULL)
    {
        return NULL_PTR;
    }

    do
    {
        
        /* 判断合法性 */
        if(minSize < 0 || maxSize <= 0 || minSize > maxSize)
        {
            minSize = DEFAULT_MIN_THREAD_NUM;
            maxSize = DEFAULT_MAX_THREAD_NUM;
        }

        /* 更新线程池属性 */
        pool->minSize = minSize;
        pool->maxSize = maxSize;
        pool->busySize = 0;
        

        /* 队列大小合法性 */
        if(queueCapacity <= 0)
        {
            queueCapacity = DEFAULT_QUEUE_MAX_SIZE;
        }
        pool->queueCapacity = queueCapacity;
        pool->queueSize = 0;
        pool->queueFront = 0;
        pool->queueRear = 0;
        pool->taskQueue = (task_t *)malloc(sizeof(task_t) * queueCapacity);
        if(pool->taskQueue == NULL)
        {
            perror("malloc error");
            break;
        }
        memset(pool->taskQueue, 0, sizeof(task_t) * queueCapacity);
        

        /* 为线程ID分配空间 */
        pool->threadID = (pthread_t *)malloc(sizeof(pthread_t) * maxSize);
        if(pool->threadID == NULL)
        {
            perror("malloc error");
            return MALLOC_ERROR;
        }
        /* 清除脏数据 */
        memset(pool->threadID, 0, sizeof(pthread_t) * maxSize);

        int ret = 0;
        /* 创建管理线程 */
        ret = pthread_create(&pool->managerID, NULL, mangerHander, pool);
        if(ret != 0)
        {
            perror("pthread_create error");
            break;
        }
        /* 创建线程 */
        for(int idx = 0; idx < minSize; idx++)
        {
            /* 判断ID号是否能够使用 */
            if(pool->threadID[idx] == 0)
            {
                ret = pthread_create(&pool->threadID[idx], NULL, threadHander, pool);
                if(ret != 0)
                {
                    // perror("pthread_create error");
                    printf("pthread_create error\n");
                    
                    break;
                }
            }

        }
        if(ret != 0)
        {
            break;
        }
        /* 存活的线程数等于最小线程数 */
        pool->aliveSize = minSize;
        /* 退出线程数 */
        pool->exitSize = 0;
        /* 销毁标志 */
        pool->shutdown = 0;

        /* 锁初始化 */
        pthread_mutex_init(&(pool->mutex), NULL);
        pthread_mutex_init(&(pool->busyMutex), NULL);
        /* 条件变量初始化 */
        if(pthread_cond_init(&(pool->notEmpty), NULL) != 0 || pthread_cond_init(&(pool->notFull), NULL) != 0)
        {
            perror("pthread_cond_init error");
            break;
        }

        return SUCCESS;
    }while(0);

    /* 程序执行到此，上面一定有bug */
    /* 回收堆空间 */
    if(pool->taskQueue != NULL)
    {
        free(pool->taskQueue);
        pool->taskQueue = NULL;
    }

    /* 回收线程资源 */
    for(int idx = 0; idx < pool->minSize; idx++)
    {
        if(pthread_join(pool->threadID[idx], NULL) != 0)
        {
            pthread_join(pool->threadID[idx], NULL);
        }
    }
    if(pool->threadID != NULL)
    {
        free(pool->threadID);
        pool->threadID = NULL;
    }
    /* 回收管理者资源 */
    if(pool->managerID != 0)
    {
        pthread_join(pool->managerID, NULL);
    }


    /* 释放锁资源 */
    if(pthread_mutex_destroy(&(pool->mutex)) != 0)
    {
        perror("pthread_mutex_destroy error");
        return MUTEX_DESTROY_ERROR;
    }
    if(pthread_mutex_destroy(&(pool->busyMutex)) != 0)
    {
        perror("pthread_mutex_destroy error");
        return MUTEX_DESTROY_ERROR;
    }
    /* 释放条件变量资源 */
    pthread_cond_destroy(&(pool->notEmpty));
    pthread_cond_destroy(&(pool->notFull));

    return UNKNOWN_ERROR;
}

/* 线程池添加任务 */
int threadPoolAddTask(threadPool_t *pool, void *(*function)(void *), void *arg)
{
    if(pool == NULL || function == NULL || arg == NULL)
    {
        return NULL_PTR;
    }

    /* 加锁 */
    pthread_mutex_lock(&pool->mutex);
    /* 任务队列满了 */
    // while(pool->aliveSize == pool->queueCapacity)
    while(pool->queueSize >= pool->queueCapacity)
    {
        pthread_cond_wait(&pool->notFull, &pool->mutex);
    }
    /* 到此位置说明队列有空位 */
    /* 添加任务 */
    pool->taskQueue[pool->queueRear].function = function;
    pool->taskQueue[pool->queueRear].arg = arg;
    /* 队列尾指针后移 */
    pool->queueRear = (pool->queueRear + 1) % pool->queueCapacity;
    /* 队列大小+1 */
    pool->queueSize++;
    /* 解锁 */
    pthread_mutex_unlock(&pool->mutex);

    /* 唤醒消费者 */
    pthread_cond_signal(&pool->notEmpty);

    return SUCCESS;
}

/* 线程池销毁 */
int threadPoolDestroy(threadPool_t *pool)
{

    pool->shutdown = 1;

    /* 回收管理者线程 */
    pthread_join(pool->managerID, NULL);
    
    /* 唤醒所有线程 */
    pthread_cond_broadcast(&pool->notEmpty);

    /* 等待所有线程退出 */
    for (int idx = 0; idx < pool->maxSize; idx++)
    {
        if (pool->threadID[idx] != 0)
        {
            pthread_join(pool->threadID[idx], NULL);
        }
    }

    return SUCCESS;
}