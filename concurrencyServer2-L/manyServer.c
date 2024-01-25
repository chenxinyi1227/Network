#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "threadPool.h"


/* 状态码 */
enum STATUS_CODE
{
    SUCCESS = 0,        // 成功
    FAILURE = -1,       // 失败
    ERROR = -2,         // 错误
    EXIT = -3,          // 退出
    TIMEOUT = -4,       // 超时
    NOT_FOUND = -5,     // 未找到
    DUPLICATE = -6,     // 重复
};


#define SERVER_PORT 8080        // 服务器端口号
#define MAX_LISENT_NUM 128      // 最大监听数
#define MAX_BUFFER_SIZE 1024    // 最大缓冲区大小
#define MIN_POLL_NUM 2          // 最小线程池数量
#define MAX_POLL_NUM 8          // 最大线程池数量
#define MAX_QUEUE_NUM 50      // 最大队列数量

/* 回收资源 */
void sigHander()
{
    /* todo */
}

/* 线程处理 */
void* threadFunc(void* arg)
{
    /* 线程分离 */
    pthread_detach(pthread_self());
    int sockfd = *(int*)arg;
    char recvbuffer[MAX_BUFFER_SIZE] = {0};
    char sendbuffer[MAX_BUFFER_SIZE] = {0};

    int readBytes = 0;
    while (1)
    {
        readBytes = read(sockfd, recvbuffer, MAX_BUFFER_SIZE);
        if (readBytes == -1)
        {
            perror("recv error");
            break;
        }
        else if (readBytes == 0)
        {
            printf("client close\n");
            close(sockfd);
            break;
        }
        else
        {
            /* 读取的字符串 */
            printf("readBytes = %d\n", readBytes);
            printf("recv data:%s\n", recvbuffer);
        }
        
        /* 处理数据 */
        sleep(2);


        /* 发送数据 */
        printf("准备发送\n");
        if (strncmp(recvbuffer, "Hello World", strlen("Hello World")) == 0)
        {
            strncpy(sendbuffer, "Good Night World", MAX_BUFFER_SIZE);  
        }
        else if (strncmp(recvbuffer, "Hello Server", strlen("Hello Server")) == 0)
        {
            strncpy(sendbuffer, "Hello Client", MAX_BUFFER_SIZE);
            sleep(2);
            break;
        }
        int ret = send(sockfd, sendbuffer, sizeof(sendbuffer), 0);
        if (ret == -1)
        {
            perror("send error");
            return NULL;
        }
        printf("send data: %s\n", sendbuffer);
        memset(recvbuffer, 0, MAX_BUFFER_SIZE);
        memset(sendbuffer, 0, MAX_BUFFER_SIZE);
    }
    
    pthread_exit(NULL);
    return NULL;

}

int main()
{
    /* 初始化线程池 */
    threadPool_t poll;
    threadPoolInit(&poll, MIN_POLL_NUM, MAX_POLL_NUM, MAX_QUEUE_NUM);

    /* 注册信号 */
    signal(SIGINT,sigHander);
    signal(SIGQUIT,sigHander);
    signal(SIGSTOP,sigHander);

    /* 创建socket套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        return FAILURE;
    }

    /* 设置socket套接字选项 *//* 设置端口复用 */
    int opt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1)
    {
        perror("setsockopt error");
        return FAILURE;
    }

    /* 绑定地址 */
    struct sockaddr_in addr;                
    // 也可以是struct sockaddr addr 但这个结构体比较难
    /* 清除脏数据 */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;                          // 指定类型为ipv4 一般都是这个 不用修改
    addr.sin_port = htons(SERVER_PORT);                 // 端口号 配置范围是1~65535; htons将主机字节序转换为网络字节序(转成大端序)

    addr.sin_addr.s_addr = htonl(INADDR_ANY);                  // INADDR_ANY:指定地址为0.0.0.0的地址 即“所有地址”、“任意地址”

    ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("bind error");
        return FAILURE;
    }

    /* 监听 */
    ret = listen(sockfd, MAX_LISENT_NUM);
    if (ret == -1)
    {
        perror("listen error");
        return FAILURE;
    }

    /* 接收连接 */
    printf("accepting...\n");
    struct sockaddr_in client_addr;                     // 用来存放客户端的地址信息
    memset(&client_addr, 0, sizeof(client_addr));       // 清除脏数据
    socklen_t client_addr_len = sizeof(client_addr);    // 用来存放客户端地址信息的长度


    char buffer[MAX_BUFFER_SIZE] = {0};     // 缓冲区
    char replyBuffer[MAX_BUFFER_SIZE] = {0};// 缓冲区
    int recv_len = 0;                       // 接收长度
    /* 循环处理线程 */
    while(1)
    {
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);      // 返回客户端套接字描述符，如果出错则返回-1，并设置errno为错误代码。
        if (client_sockfd == -1)
        {
            perror("accept error");
            return FAILURE;
        }

        // /* 开线程 */    /* 没来一个请求开辟一个线程 */
        // pthread_t tid;
        // pthread_create(&tid, NULL, threadFunc, (void*)&client_sockfd);
        // sleep(1);

        /* 添加到任务队列 */
        threadPoolAddTask(&poll, threadFunc, (void*)&client_sockfd);

    }
    /* 释放线程池 */
    threadPoolDestroy(&poll);


    printf("准备关闭\n");
    /* 关闭连接 */
    // close(client_sockfd);
    close(sockfd);
    printf("server exit\n");

    return 0;
}