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
#include <json-c/json.h>
#include <json-c/json_object.h>

#define BUFFER_SIZE 128
#define SERVER_PORT 9999
#define SERVER_IP   "172.25.23.103"


int main()
{
    /* 新建json对象 */
    struct json_object * object = json_object_new_object();
    if (object == NULL)
    {
        /* todo... */
    }

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

    int ret = inet_pton(AF_INET, SERVER_IP, (void *)&sockAddress.sin_addr.s_addr);
    if (ret == -1)
    {
        perror("inet_pton error");
        exit(-1);
    }
    

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, 0, sizeof(recvBuffer));

    /* 1: 注册 2.登陆 */
    /* { "way" : 1, "name" : "zhangsan", "passwd" : "abd123" } */

    struct json_object * wayVal = json_object_new_int64(1);
    json_object_object_add(object, "way", wayVal);

    /* 将json对象转成字符串 */
    const char * ptr = json_object_to_json_string(object);

    /* 发送字符串 */
    sendto(sockfd, ptr, strlen(ptr), 0, (struct sockaddr*)&sockAddress, sizeof(sockAddress));

    
    struct sockaddr serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    socklen_t sockAddressLen = sizeof(serverAddress);
    recvfrom(sockfd, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr*)&serverAddress, &sockAddressLen);
    printf("recvBuffer: %s\n", recvBuffer);

    /* 关闭通信句柄 */
    close(sockfd);

    /* 释放json对象 */
    json_object_put(object);
    return 0;

}