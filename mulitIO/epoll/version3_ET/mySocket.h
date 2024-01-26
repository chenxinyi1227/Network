#ifndef _SOCKET_H_
#define _SOCKET_H_

int createSocket(int *sockfd);
int bindSocket(int sockfd);
int listenSocket(int sockfd);
int Epoll_socket(int *epfd);

#endif