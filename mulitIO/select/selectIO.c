#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>
#include <error.h>
#include <sys/select.h>
#include <ctype.h>


#define SERVER_PORT 8080
#define MAX_LISTEN  128
#define LOCAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 128


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

    /* 将监听的sockfd的状态委托给内核检测 */
    int maxfd = sockfd;
    /* 初始化检测的读集合 */
    fd_set readSet;
    fd_set readTmp;

    /* 清除脏数据 */
    FD_ZERO(&readSet);
    /* 将监听的sockfd设置到检测的读集合中. */
    FD_SET(sockfd, &readSet);

    

    /* 读缓冲区 */
    char readBuffer[BUFFER_SIZE];
    memset(readBuffer, 0, sizeof(readBuffer));

    int readBytes = 0;
    while (1)
    {
        /* 实时更新 */
        /* readTmp 是委托内核检测的所有文件描述符. */
        readTmp = readSet;
        
        /* 默认阻塞 */
        int num = select(maxfd + 1, &readTmp, NULL, NULL, NULL);
        /* readTmp 中的数据被内核改写(传入传出参数), 只保留发生变化的文件描述符的标识位的1, 没变化的改为0 */
        /* 只要readTmp中的fd对应的标志位为1 -> 缓冲区有数据 */
        
        if (FD_ISSET(sockfd, &readTmp))
        {
            /* 有新连接, 就需要调用accept函数 */

            /* 客户的信息 */
            struct sockaddr_in clientAddress;
            memset(&clientAddress, 0, sizeof(clientAddress));

            socklen_t clientAddressLen = 0;
            int acceptfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLen);
            if (acceptfd == -1)
            {
                perror("accpet error");
                exit(-1);
            }

            /* 将通信的文件描述符添加到读集合中 */
            FD_SET(acceptfd, &readSet);
        }
       
        for (int idx = 0; idx < maxfd + 1; idx++)
        {
            /* 清除脏数据 */
            memset(readBuffer, 0, sizeof(readBuffer));
            /* 判断从监听的文件描述符之后到maxfd这个范围内的文件描述符是否读缓冲区有数据 */
            if (idx != sockfd && FD_ISSET(idx, &readTmp))
            {
                readBytes = read(idx, (void *)readBuffer, sizeof(readBuffer));
                if (readBytes < 0)
                {
                    perror("read eror");
                    close(idx);
                    continue;
                }
                else if (readBytes == 0)
                {
                    printf("客户端下线了...\n");
                    /* 将检测的文件描述符从读集合中删除 */
                    FD_CLR(idx, &readSet);
                    /* 关闭通信句柄 */
                    close(idx);
                    continue;
                }
                else
                {
                    #if 1
                    /* 读到的字符串 */
                    for (int idx = 0; idx < readBytes; idx++)
                    {
                        readBuffer[idx] = toupper(readBuffer[idx]);
                    }
                    /* 发送数据 */
                    write(idx, readBuffer, strlen(readBuffer) + 1);
                    #endif
                } 
            }
        }
    }

    /* 关闭文件描述符 */
    close(sockfd);

    return 0;
}