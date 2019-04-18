//
// Created by duoml on 19-4-12.
//

#include "include.h"
#include <poll.h>
//int main()
int test_poll()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = INADDR_ANY;
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (sockaddr *)&addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);
    struct sockaddr_in clientaddr;
    socklen_t cli_addrlen = sizeof(clientaddr);
    int connfd = accept(listenfd, (sockaddr *)&clientaddr, &cli_addrlen);

    char recvbuf[1024];
    char sendbuf[1024];
    struct pollfd pfd[1];
    bzero(pfd, sizeof(pfd));
    pfd->fd = connfd;
    pfd->events = POLLIN | POLLPRI | POLLRDHUP ;
    while(1)
    {
        memset(recvbuf, '\0', sizeof(recvbuf));
        memset(sendbuf, '\0', sizeof(sendbuf));
        ret = poll(pfd, 1, -1);
        if(ret < 0)
        {
            printf("selection failure\n");
            break;
        }
        if(pfd->revents & POLLRDHUP)
        {
            printf("conn close\n");
            break;
        }
        else if(pfd->revents & POLLIN)
        {
            ret = recv(connfd, recvbuf, sizeof(recvbuf) - 1, 0);
            printf("get %d bytes of normal data: %s\n", ret, recvbuf);
            strcpy(sendbuf, "OK");
            ret = send(connfd, sendbuf, strlen(sendbuf), 0);
            if(ret <= 0)
            {
                break;
            }
        }
        else if(pfd->revents & POLLPRI)
        {
            ret = recv(connfd, recvbuf, sizeof(recvbuf) - 1, MSG_OOB);
            printf("get %d bytes of oob data: %s\n", ret, recvbuf);
            strcpy(sendbuf, "OK");
            ret = send(connfd, sendbuf, strlen(sendbuf), MSG_OOB);
            if(ret <= 0)
            {
                break;
            }
        }
    }
    close(connfd);
    close(listenfd);
    return 0;
}