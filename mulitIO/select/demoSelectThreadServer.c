#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

/* 互斥锁 */
pthread_mutex_t mutex;

#define SERVER_PORT 8080
#define MAX_LISTEN  128

#define BUFFER_SIZE     128

/* 多线程 实现并发 */
typedef struct FDINFO
{   
    /* 句柄 */
    int fd;
    /* 最大的文件描述符 */
    int *maxfd;
    /* 读集合 */
    fd_set *readSet;
} FDINFO;

void *accept_func(void *arg)
{
    FDINFO * info = (FDINFO *)arg;
    /* 线程分离 */
    pthread_detach(pthread_self());

    int sockfd = info->fd;
    int acceptfd = accept(sockfd, NULL, NULL);
    if (acceptfd == -1)
    {
        perror("accpet error");
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&mutex);
    /* 将通信的句柄 放到读集合 */
    FD_SET(acceptfd, info->readSet);

    /* 更新maxfd的值 */
    /* 解引用 */
    *(info->maxfd) = *(info->maxfd) < acceptfd ? acceptfd : *(info->maxfd);
    /* 解锁 */
    pthread_mutex_unlock(&mutex);

    /* 释放堆空间, 避免内存泄露 */
    if (info)
    {
        free(info);
        info = NULL;
    }
    /* 线程退出 */
    pthread_exit(0);
}


void * comm_func(void * arg)
{   
    FDINFO * info = (FDINFO *)arg;
    /* 线程分离 */
    pthread_detach(pthread_self());

    int connfd = info->fd;

    char buffer[BUFFER_SIZE];
    /* 清除脏数据 */
    bzero(buffer, sizeof(buffer));
    /* 程序到这里, 一定有通信(老客户) */
    int readBytes = read(connfd, buffer, sizeof(buffer) - 1);
    if (readBytes < 0)
    {
        perror("read error");
        /* 加锁 */
        pthread_mutex_lock(&mutex);
        /* 将该通信句柄从监听的读集合中删掉 */
        FD_CLR(connfd, (info->readSet));
        /* 解锁 */
        pthread_mutex_unlock(&mutex);
        /* 关闭文件句柄 */
        close(connfd);
    }
    else if (readBytes == 0)
    {
        printf("客户端断开连接...\n");
        /* 加锁 */
        pthread_mutex_lock(&mutex);
        /* 将该通信句柄从监听的读集合中删掉 */
        FD_CLR(connfd, info->readSet);
        /* 解锁 */
        pthread_mutex_unlock(&mutex);
        /* 关闭通信句柄 */
        close(connfd);
    }
    else
    {
        printf("recv:%s\n", buffer);

        for (int jdx = 0; jdx < readBytes; jdx++)
        {
            buffer[jdx] = toupper(buffer[jdx]);
        }

        /* 发回客户端 */
        write(connfd, buffer, readBytes);
        usleep(500);
    }

    /* 释放堆空间, 避免内存泄露 */
    if (info)
    {
        free(info);
        info = NULL;
    }

    /* 线程退出 */
    pthread_exit(NULL);
}

int main()
{   
    /* 初始化互斥锁 */
    pthread_mutex_init(&mutex, NULL);

    /* 创建套接字 句柄 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }
    printf("sockfd:%d\n", sockfd);

    
    /* 将本地的IP和端口绑定 */
    struct sockaddr_in localAddress;
    bzero((void *)&localAddress, sizeof(localAddress));
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(SERVER_PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    
    socklen_t localAddressLen = sizeof(localAddress);
    int ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    fd_set readSet;
    /* 清空集合 */
    FD_ZERO(&readSet);
    /* 把监听的文件描述符添加到读集合中, 让内核帮忙检测 */
    FD_SET(sockfd, &readSet);

    int maxfd = sockfd;

#if 0
    /* 设置超时 */
    struct timeval timeValue;
    bzero(&timeValue, sizeof(timeValue));
    timeValue.tv_sec = 5;
    timeValue.tv_usec = 0;
#endif

    fd_set tmpReadSet;
    /* 清除脏数据 */
    bzero(&tmpReadSet, sizeof(tmpReadSet));
    while (1)
    {
        /* */
        pthread_mutex_lock(&mutex);
        /* 备份读集合 */
        tmpReadSet = readSet;
        pthread_mutex_unlock(&mutex);

        ret = select(maxfd + 1, &tmpReadSet, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select error");
            break;
        }

        /* 如果sockfd在readSet集合里面 */
        if (FD_ISSET(sockfd, &tmpReadSet))
        {
            FDINFO * info = (FDINFO *)malloc(sizeof(FDINFO) * 1);
            if (info == NULL)
            {
                perror("malloc error");
                exit(-1);
            }
            info->fd = sockfd;
            info->maxfd = &maxfd;
            info->readSet = &readSet;
            pthread_t tid;
            ret = pthread_create(&tid, NULL, accept_func, (void *)info);
            if (ret == -1)
            {
                perror("thread create error");
                exit(-1);
            }
        }
        
        /* 程序到这个地方: 说明可能有通信 */
        for (int idx = 3; idx <= maxfd; idx++)
        {
            if (idx != sockfd && FD_ISSET(idx, &tmpReadSet))
            {
                FDINFO * info = (FDINFO *)malloc(sizeof(FDINFO) * 1);
                if (info == NULL)
                {
                    perror("malloc error");
                    exit(-1);
                }
                info->fd = idx;
                info->readSet = &readSet;

                pthread_t tid;
                ret = pthread_create(&tid, NULL, comm_func, (void *)info);
                if (ret == -1)
                {
                    perror("thread create error");
                    exit(-1);
                }
            }
        }

    
    }

    /* 关闭文件描述符 */
    close(sockfd);



    /* 销毁互斥锁 */
    pthread_mutex_destroy(&mutex);

    return 0;
}