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
#include <ctype.h>

#define BUFFER_SIZE 128
#define SERVER_PORT 9999

int main()
{
    /* 创建套接字的句柄 */
    int sockfd = socket(AF_INET, SOCK_DGRAM , 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    struct sockaddr_in sockAddress;
    memset(&sockAddress, 0, sizeof(sockAddress));
    sockAddress.sin_family = AF_INET;
    sockAddress.sin_port = htons(SERVER_PORT);
    #if 1
    sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    #else
    inet_pton(AF_INET, "41.12.56.123", (void *)&sockAddress.sin_addr.s_addr);
    #endif

    int ret = bind(sockfd, (struct sockaddr *)&sockAddress, sizeof(sockAddress));
    if (ret == -1)
    {
        perror("bind error");
        exit(-1);
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int readBytes = 0;
    
    struct sockaddr clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t clientAddressLen = sizeof(clientAddress);
    while (1)
    {
        readBytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, &clientAddress, &clientAddressLen);
        if (readBytes > 0)
        {
            printf("recv buffer:%s\n", buffer);
            for (int idx = 0; idx < readBytes; idx++)
            {
                #if 0
                if (buffer[idx] >= 'a' && buffer[idx] <= 'z')
                {
                    buffer[idx] -= 32;
                }
                #else
                buffer[idx] = toupper(buffer[idx]);
                #endif
            }
        }
        sendto(sockfd, buffer, sizeof(buffer), 0, &clientAddress, clientAddressLen);
    }

    /* 关闭通信句柄 */
    close(sockfd);


    return 0;
}