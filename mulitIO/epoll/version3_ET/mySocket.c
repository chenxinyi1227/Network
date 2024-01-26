#include "mySocket.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <error.h>


#define SERVER_PORT 8080
#define MAX_LISTEN  128
#define LOCAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 128

int createSocket(int *sockfd)
{
    /* 创建socket套接字 */
    int mySocket = *sockfd;
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket == -1)
    {
        perror("socket error");
        exit(-1);
    }

    /* 设置端口复用 */
    int enableOpt = 1;
    int ret = setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, (void *)&enableOpt, sizeof(enableOpt));
    if (ret == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }
    *sockfd = mySocket;
    return 0;
}

/* 绑定 */
int bindSocket(int sockfd)
{
    struct sockaddr_in localAddress;  
    memset(&localAddress, 0, sizeof(localAddress)); /* 清除脏数据 */

    localAddress.sin_family = AF_INET;   /* 地址族 */
    localAddress.sin_port = htons(SERVER_PORT); /* 端口需要转成大端 */
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* ip地址需要转成大端 */

    int localAddressLen = sizeof(localAddress);
    int ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }
    return 0;
}

/* 监听 */
int listenSocket(int sockfd)
{
    int ret = listen(sockfd, MAX_LISTEN);
    if (ret == -1)
    {
        perror("listen error");
        exit(-1);
    }
    return 0;
}