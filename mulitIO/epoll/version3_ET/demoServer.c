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
#include <sys/epoll.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_PORT 8080
#define MAX_LISTEN  128
#define LOCAL_IPADDRESS "127.0.0.1"
#define BUFFER_SIZE 5

#define EVENT_SIZE  1024

int main()
{
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

    /* 创建epoll 红黑树 */
    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll create error");
        exit(-1);
    }


    /* 将sockfd 添加到监听事件中 */
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = sockfd;
    event.events = EPOLLIN | EPOLLET; 

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl error");
        exit(-1);
    }
    
    struct epoll_event events[EVENT_SIZE];
    /* 清除脏数据 */
    memset(events, 0, sizeof(events));
    int maxEventSize = sizeof(events) / sizeof(events[0]);
    while (1)
    {
        int num = epoll_wait(epfd, events, maxEventSize, -1);
        if (num == -1)
        {
            perror("epoll wait error");
            exit(-1);
        }
        /* 程序到这里一定有通信 */
        printf("num = %d\n", num);

        for (int idx = 0; idx < num; idx++)
        {
            int fd = events[idx].data.fd;
            if (fd == sockfd)
            {
                int acceptfd = accept(sockfd, NULL, NULL);
                if (acceptfd == -1)
                {
                    perror("accpet error");
                    exit(-1);
                }
                /* 将通信的accept句柄 设置成非阻塞 */
                int flag = fcntl(acceptfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(acceptfd, F_SETFL, flag);

                /* 将通信句柄放到epoll的红黑树上 */
                struct epoll_event event;
                event.data.fd = acceptfd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd, EPOLL_CTL_ADD, acceptfd, &event);
            }
            else
            {
                char buffer[BUFFER_SIZE] = { 0 };
                while (1)
                {
                    int readBytes = read(fd, (void *)&buffer, sizeof(buffer) - 1);
                    if (readBytes < 0)
                    {
                        if (errno == EAGAIN)
                        {
                            /* 内核缓冲区数据接收完成 */
                            printf("数据已经接收完成");
                            break;
                        }
                        else
                        {
                            perror("read eror");
                            /* 从epoll的红黑树上删除通信结点 */
                            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                            close(fd);
                            break;
                        }
                    }
                    else if (readBytes == 0)
                    {
                        printf("客户端下线了...\n");
                        /* 从epoll的红黑树上删除通信结点 */
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                        break;
                    }   
                    else
                    {
                        printf("recv:%s\n", buffer);

                        for (int jdx = 0; jdx < readBytes; jdx++)
                        {
                            buffer[jdx] = toupper(buffer[jdx]);
                        }

                        /* 发回客户端 */
                        write(fd, buffer, readBytes);
                        usleep(500);
                    } 
                }
            }

        }
    }

    /* 关闭文件描述符 */
    close(sockfd);

    return 0;
}