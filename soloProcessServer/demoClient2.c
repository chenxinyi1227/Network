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


#define SERVER_PORT 8080
#define SERVER_IP   "172.25.23.103"
#define BUFFER_SIZE 128

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        exit(-1);
    }

    
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    /* 端口 */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    /* IP地址 */
    int ret = inet_pton(AF_INET, SERVER_IP, (void *)&(serverAddress.sin_addr.s_addr));
    if (ret != 1)
    {
        perror("inet_pton error");
        exit(-1);
    }

    /* ip地址 */
    ret = connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (ret == -1)
    {
        perror("connect error");
        exit(-1);
    }

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    while (1)
    {
#if 1
        strncpy(buffer, "778899", sizeof(buffer) - 1);
        write(sockfd, buffer, sizeof(buffer));

        read(sockfd, recvBuffer, sizeof(recvBuffer));
        printf("recv:%s\n", recvBuffer);
#else

        int num = 0X12345678;
        write(sockfd, (void *)&num, sizeof(num));

        sleep(5);
#endif
    }
    
    
    /* 休息5S */
    sleep(5);

    
    close(sockfd);

    return 0;
}