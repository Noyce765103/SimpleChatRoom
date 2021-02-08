#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

//save all clients' socket
list<int> clients_list;

//server ip
#define SERVER_IP "127.0.0.1"

//server port
#define SERVER_PORT 9873

//epoll size 
#define EPOLL_SIZE 5000

//message buffer size
#define BUF_SIZE 1024*8

#define SERVER_WELCOME "Welcome to join in the char room! Your char ID is: Client #%d"

#define SERVER_MESSAGE "ClientID %d say >> %s"

#define EXIT "EXIT"

#define CAUTION "There is only on in the char room!"

int setnonblocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK);
    return 0;
}

void addfd(int epollfd, int fd, bool enable_et)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(enable_et)   ev.events |= EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
}

int sendBroadcastmessage(int clientfd)
{
    char buf[BUF_SIZE], message[BUF_SIZE];
    memset(buf, '\0', BUF_SIZE);
    memset(message, '\0', BUF_SIZE);

    //recive message
    printf("read from client(clientID = %d)\n", clientfd);
    int len = recv(clientfd, buf, BUF_SIZE, 0);
    if(len == 0)
    {
        close(clientfd);
        clients_list.remove(clientfd);
        printf("ClientId = %d closed. \n There are %d client in the char room now\n", clientfd, (int)clients_list.size());
    }
    else
    {
        if(clients_list.size() == 1){
            send(clientfd, CAUTION, strlen(CAUTION), 0);
            return len;
        }
        sprintf(message, SERVER_MESSAGE, clientfd, buf);
        for(auto it:clients_list){
            if(it != clientfd){
                if(send(it, message, BUF_SIZE, 0) < 0) {perror("error"); exit(-1);}
            }
        }
    }
    return len;
}

#endif