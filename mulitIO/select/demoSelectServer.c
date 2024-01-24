#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>//bzreo
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>//toupper

#define SERVER_PORT 8080
#define MAX_LISTRN 128
#define BUFFER_SIZE 1024

/* 用单进程/线程 实现并发 */
int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);//最小文件描述符为3
    if(sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    int opt = 1;
    int retopt = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
    if(retopt == -1)
    {
        perror("setsockopt error");
        exit(-1);
    }

    /* 将本地的IP和端口绑定 */
    struct sockaddr_in localAddress;
    bzero((void*)&localAddress, sizeof(localAddress));
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(SERVER_PORT);//转成大端
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);//接受任意连接
    // localAddress.sin_addr.s_addr = inet_addr(INADDR_ANY);
    socklen_t localAddressLen = sizeof(localAddress);
    int retbind = bind(sockfd, (struct sockaddr *)&localAddress, localAddressLen);
    if(retbind == - 1)
    {
        perror("bind error");
        exit(-1);
    }
    
    /* 监听 */
    int retlisten = listen(sockfd, MAX_LISTRN);
    if(retlisten == -1)
    {
        perror("listen error");
        exit(-1);
    }

    fd_set readset;
    //清空集合
    FD_ZERO(&readset);
    /* 把监听的文件描述符添加到读集合中，然内核帮忙检测 */
    FD_WET(sockfd, &readset);
    //一定要放到循环里
    int maxfd = sockfd;

    #if 0
    /* 超时 */
    struct timeVal timeValue;
    #endif

    while(1)
    {
        int retSelect = select(maxfd + 1, &readset, NULL, NULL, NULL);
        if(retSelect == -1)
        {
            perror("select error");
            break;
        }

        /* 如果sockfd在readSet集合里面 */
        if(FD_ISSET(sockfd, &readset))
        {   
            // struct sockaddr * c_addr;
            // bzero((void*)&c_addr, sizeof(c_addr));
            int acceptfd = accept(sockfd, NULL, NULL);
            if(acceptfd == -1)
            {
                perror("accept error");
                exit(-1);
            }
            //将通信的句柄 放到读集合中
            FD_SET(acceptfd, &readset);
            //更新maxfd的值
            maxfd = maxfd < acceptfd ? acceptfd : maxfd;
            //maxfd = accept 
        }

        /* 程序到这个地方：说明可能有通信 */
        for(int idx = 0; idx <= maxfd; idx++)
        {
            if(idx != sockfd && FD_ISSET(idx, &readset))
            {
                char buffer[BUFFER_SIZE];
                bzero(buffer, sizeof(buffer));
                /* 程序到这里，一定通信（老客户） */
                int readBytes = read(idx, buffer, sizeof(buffer) - 1);
                if(readBytes < 0)
                {
                    perror("read error");
                    //将该fd通信句柄从监听的读集合中删除
                    FD_CLR(idx, &readset);
                    //关闭文件句柄
                    close(idx);
                    //这边要做成continue...让下一个已read的fd句柄进行通信
                    continue;
                }
                else if(readBytes == 0)
                {
                    printf("客户端断开连接\n");
                    //将该fd通信句柄从监听的读集合中删除
                    FD_CLR(idx, &readset);
                    //关闭通信句柄
                    close(idx);
                    continue;
                }
                else
                {           
                    printf("buffer:%s\n", buffer);
                    for(int jdx = 0; jdx < 0; jdx++)
                    {
                        buffer[jdx] = toupper(buffer[jdx]);
                    }
                    //发回客户端
                    write(idx, buffer, readBytes);
                    usleep(500);
                }
            }
        }
    }

    /* 关闭文件描述符 */
    close(sockfd);
    return 0;
}