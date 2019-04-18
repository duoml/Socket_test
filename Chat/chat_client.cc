//
// Created by duoml on 19-4-15.
//
#include "include.h"
#include <poll.h>
#define BUFFER_SIZE 64
int main()
{
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(2222);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("connection failed\n");
        close(sockfd);
        return 0;
    }
    pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP;
    fds[1].revents = 0;
    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert (ret != -1);
    while(1)
    {
        ret = poll(fds, 2, -1);
        if(ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        if(fds[1].revents & POLLRDHUP)
        {
            printf("server close connection\n");
            break;
        }
        else if(fds[1].revents & POLLIN)
        {
            memset(read_buf, 0, BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }
        if(fds[0].revents & POLLIN)
        {
            splice(0,NULL,pipefd[1],NULL,32768,SPLICE_F_MOVE|SPLICE_F_MORE);
            splice(pipefd[0],NULL,sockfd,NULL,32768,SPLICE_F_MORE |SPLICE_F_MOVE);
        }
    }
    close(sockfd);
    return 0;
}
