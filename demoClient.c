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


#define SERVER_PORT 8080
#define SERVER_IP   "172.18.188.222"
#define BUFFER_SIZE 128

int main()
{
    /* 新建json对象 */
    struct json_object * object = json_object_new_object();
    if(object == NULL)
    {
        /* stdo... */
    }

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

    /* 1、注册 2、登录 */
    /* {"way" : 1， “name":"zhangsan", "passord":"1213"} */

    struct json_object * wayVal = json_object_new_int64(1);//登录方式
    json_object_object_add(object, "way", wayVal);
    /* 将json对象转成字符串 */
    const char * ptr = json_object_to_json_string(object);

    
    while (1)
    {
        strncpy(buffer, "加油 254", sizeof(buffer) - 1);
        //发送字符串
       // sendto(sockfd, ptr, strlen(str), 0, (struct so));
        // write(sockfd, buffer, sizeof(buffer));

        read(sockfd, recvBuffer, sizeof(recvBuffer) - 1);
        printf("recv:%s\n", recvBuffer);
    }
    
    
    /* 休息5S */
    sleep(5);

    
    close(sockfd);

    json_object_put(object);
    return 0;
}