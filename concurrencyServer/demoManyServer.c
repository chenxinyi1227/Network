#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <error.h>
#include <pthread.h>


#define SERVER_PORT 8080
#define MAX_LISTEN  128
#define LOCAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 128

// void sigHander(int sigNum)
// {
//     int ret = 0;

//     /* 资源回收 */
//     /* todo... */

// }


/* 线程处理函数 */
void * threadHandle(void *arg)
{
    /* 设置线程分离 */
    pthread_detach(pthread_self());

    /* 通信句柄. */
    int acceptfd = *(int *)arg;

    /* 通信 */

    /* 接收缓冲区 */
    char recvbuffer[BUFFER_SIZE];
    memset(recvbuffer, 0, sizeof(recvbuffer));

    /* 发送缓冲区 */
    char sendBuffer[BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));

    /* 读取到的字节数 */
    int readBytes = 0;
    while (1)
    {
        readBytes = read(acceptfd, (void *)&recvbuffer, sizeof(recvbuffer));
        if (readBytes <= 0)
        {
            perror("read eror");
            close(acceptfd);
            break;
        }
        else
        {
            /* 读到的字符串 */
            printf("buffer:%s\n", recvbuffer);
            if (strncmp(recvbuffer, "123456", strlen("123456")) == 0)
            {
                strncpy(sendBuffer, "一起加油123456", sizeof(sendBuffer) - 1);
                sleep(1);
                write(acceptfd, sendBuffer, sizeof(sendBuffer));
            }
            else if (strncmp(recvbuffer, "778899", strlen("778899")) == 0)
            {
                strncpy(sendBuffer, "一起加油778899", sizeof(sendBuffer) - 1);
                sleep(1);
                write(acceptfd, sendBuffer, sizeof(sendBuffer));
            }
        }    
    }

    /* 线程退出 */
    pthread_exit(NULL);
}

int main()
{
    /* 信号注册 */
    // signal(SIGINT, sigHander);
    // signal(SIGQUIT, sigHander);
    // signal(SIGTSTP, sigHander);

    /* 创建socket套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    /* 设置端口复用 */
    int enableOpt = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }

    /* 绑定 */
#if 0
    /* 这个结构体不好用 */
    struct sockaddr localAddress;
#else
    struct sockaddr_in localAddress;
#endif
    /* 清除脏数据 */
    memset(&localAddress, 0, sizeof(localAddress));

    /* 地址族 */
    localAddress.sin_family = AF_INET;
    /* 端口需要转成大端 */
    localAddress.sin_port = htons(SERVER_PORT);
    /* ip地址需要转成大端 */

    /* Address to accept any incoming messages.  */
    /* INADDR_ANY = 0x00000000 */
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY); 

    
    int localAddressLen = sizeof(localAddress);
    ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
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

    /* 客户的信息 */
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));

    
    /* 循环去接收客户端的请求 */
    while (1)
    {
        socklen_t clientAddressLen = 0;
        int acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
        if (acceptfd == -1)
        {
            perror("accpet error");
            exit(-1);
        }

#if 1
        pthread_t tid;
        /* 开一个线程去服务acceptfd */
        ret = pthread_create(&tid, NULL, threadHandle, (void *)&acceptfd);
        if (ret != 0)
        {
            perror("thread create error");
            exit(-1);
        }

        
#endif
    }

    /* 关闭文件描述符 */
    close(sockfd);

    return 0;
}
