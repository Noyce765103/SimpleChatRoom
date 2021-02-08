#include "utility.h"

int main()
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    //创建监听socket
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)    { perror("listenfd"); exit(-1);}
    if(bind(listenfd, (sockaddr*)&serverAddr, sizeof(sockaddr)) < 0)
    {
        perror("bind error");
        exit(-1);
    }
    int ret = listen(listenfd, 5);
    if(ret < 0) { perror("listen error"); exit(-1); }
    int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0) { perror("epfd error"); exit(-1); }
    static struct epoll_event events[EPOLL_SIZE];
    addfd(epfd, listenfd, true);
    while(1)
    {
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if(epoll_events_count < 0)
        {
            perror("epoll failure");
            break;
        }
        for(int i = 0; i < epoll_events_count; i ++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                struct sockaddr_in clientAddr;
                socklen_t client_addrLen = sizeof(struct sockaddr_in);
                int clientfd = accept(clientfd, (sockaddr*)&clientAddr, &client_addrLen);
                printf("client connection from: %s : %d(IP : port), clientfd = %d \n",
                        inet_ntoa(clientAddr.sin_addr),
                        ntohs(clientAddr.sin_port),
                        clientfd);
                addfd(epfd, clientfd, true);

                clients_list.push_back(clientfd);
                printf("Now there are %d clients in the chart room\n", (int)clients_list.size());

                char message[BUF_SIZE];
                memset(message,'\0', BUF_SIZE);
                sprintf(message, SERVER_WELCOME, clientfd);
                int ret = send(clientfd, message, BUF_SIZE, 0);
                if(ret < 0) { perror("send error"); exit(-1); }
            }
            else if(events[i].events & EPOLLIN)
            {
                int ret = sendBroadcastmessage(sockfd);
            }
            else break;

        }
    }
    close(listenfd);
    close(epfd);
}