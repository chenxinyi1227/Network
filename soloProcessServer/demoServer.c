#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVER_PORT 8080
#define MAX_LISTEN 128
#define LCOAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 128

#if 0
typedef struct sockaddr_in 
{
    short int sin_family;          // 地址族，一般为 AF_INET
    unsigned short int sin_port;   // 端口号
    struct in_addr sin_addr;       // IP 地址
    unsigned char sin_zero[8];     // 不使用，设置为 0
}localAddress;
#endif

// void sigHander(void *arg)
// {

// }

int main()
{
    /* 注册信号 */
    // signal(SIGINT, sigHander); //ctrl + c
    // signal(SIGQUIT, sigHander);//ctrl + /
    // signal(SIGSTOP, sigHander);//ctrl + z

    /* 创建socket套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

#if 0
    /* 绑定 */
    /* 这个函数不好用 */
    struct socketaddr localAddress;
#endif
    struct sockaddr_in localAddress;

    memset(&localAddress, 0, sizeof(localAddress));

    /* 地址族 */
    localAddress.sin_family = AF_INET;
    /* 端口需要转成大端 */
    localAddress.sin_port = htons(SERVER_PORT);
    /* ip地址需要转成大端   sin_addr.s_addr这个宏一般用于本地的绑定操作*/
   
#if 1
    /* accept any message */
    /* INADDR_ANY = 0x00000000 */
    localAddress.sin_addr.s_addr = htons(INADDR_ANY);//0.0.0.0 
   
#else   
    inet_pton(AF_INET, LCOAL_IPADDRESS, &(serverAddress.sin_addr.s_addr));
#endif
    int localAddressLen = sizeof(localAddress);
    int ret = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if(ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    /* 监听 */
    listen(sockfd, MAX_LISTEN);
    if(ret == -1)
    {
        perror("listen error");
        exit(-1);
    }

    /* 客户的信息 */
    struct sockaddr_in6 clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));

    socklen_t clientAddressLen = 0;
    int acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if(acceptfd == -1)
    {
        perror("accept error");
        exit(-1);
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    
    char replyBuffer[BUFFER_SIZE];
    memset(replyBuffer, 0, sizeof(replyBuffer));

    int readBytes = 0;
    while(1)
    {
        readBytes = read(acceptfd, buffer, sizeof(buffer));
        if(readBytes <= 0)
        {
            perror("read error");
            printf("111\n");
            close(acceptfd);
            exit(-1);
        }
        else
        {
            /* 读到的字符数 */
            printf("buffer:%s\n", buffer);

            sleep(3);

            strncpy(replyBuffer, "一起加油", sizeof(replyBuffer) - 1);
            write(acceptfd, "666", strlen("666") + 1);

        }
    }
    // close(sockfd);
    /* 关闭文件描述符 */
    close(acceptfd);
    
    return 0;
}