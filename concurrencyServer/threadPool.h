#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <pthread.h>

typedef struct task_t
{
    void *(*worker_hander)(void *arg); // 钩子函数 - 回调函数 
    void *arg; // 参数
}task_t;

/* 线程池结构体 */
typedef struct threadpool_t
{
    /* 任务队列 */
    task_t * taskQueue; //里面放的是函数和参数
    int queueCapacity;  //任务队列容量
    int queueSize;      //任务队列的任务数
    int queueFront;     //任务队列的对头
    int queueRear;      //任务队列的堆尾

    /* 线程池 */
    pthread_t *threadIds;     // 线程池中的线程 
    pthread_t *managerThread; // 线程池中的管理线程

    int minThreads;      // 最小的线程数 
    int maxThreads;      // 最大的线程数 

    int busyThreadNums;     //干活的线程数 "干活的线程数 ！= 存活的线程数"
    int liveThreadNums;     //存活的线程数

    /* 锁 */
    pthread_mutex_t mutexpool;  //锁 - 维护整个线程
    pthread_mutex_t mutexBusy;  //锁 - 只维护干活线程
    pthread_cond_t notEmpty;    //条件变量:任务队列有任务可以消费
    pthread_cond_t notFull;     //条件变量：任务队列有空位，可以继续放

    int exitThreadNums; //离开的线程数
    int shutDown;       //关闭

}threadpool_t;

/* 线程池初始化 */
int threadPoolInit(threadpool_t *pool, int minThread, int maxThreads, int queueCapacity);

/* 线程增加任务 */
int threadPoolAddTask(threadpool_t *pool, void *(worker_hander)(void *), void *arg);

/* 线程池销毁 */
int threadPoolDestroy(threadpool_t *pool);

#endif // __THREAD_POOL_H_