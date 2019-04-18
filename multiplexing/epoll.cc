//
// Created by duoml on 19-4-12.
//

#include "include.h"
#include <pthread.h>
#include <sys/epoll.h>
#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 5 
struct fds
{
    int epollfd;
    int sockfd;
};
int setnoblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL,new_opt);
    return old_opt;
}

void addfd(int epollfd, int fd, bool enable_et)
{
    setnoblocking(fd);
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLPRI;
    if(enable_et)
    {
        event.events |= EPOLLET ;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}
void reset_oneshot(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLPRI | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD,fd, &event);
}

void delfd(int epollfd, int fd, bool enable_et)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLPRI;
    if(enable_et)
    {
        event.events |= EPOLLET | EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_DEL,fd, &event);
}

void* worker(void * arg)
{
    int sockfd = ((fds*)arg)->sockfd;
    int epollfd = ((fds*)arg)->epollfd;
    printf("start new thread to receive data on fd: %d\n",sockfd);
    char buf[BUFFER_SIZE];
    memset(buf,'\0',sizeof(buf));
    while(1)
    {
        int ret =recv(sockfd,buf,BUFFER_SIZE-1,0);
        if(ret == 0)
        {
            close(sockfd);
            printf("foreigner closed the connection\n");
            break;
        }
        else if(ret<0)
        {
            if(errno == EAGAIN)
            {
                reset_oneshot(epollfd,sockfd);
                printf("read leater\n");
                break;
            }
        }
        else{
            printf("get content: %s\n",buf);
            sleep(5);
        }
    }
    printf("end thread receiving data on fd: %d\n",sockfd);
}

void lt(epoll_event *events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_addrlen = sizeof(cli_addr);
            int connfd = accept(listenfd, (sockaddr *)&cli_addr, &cli_addrlen);
            addfd(epollfd, connfd, false);
        }
        else if(events[i].events & EPOLLIN)
        {
            printf("event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0)
            {
                close(sockfd);
//                continue;
                break;
            }
            printf("get %d bytes of normal content: %s\n", ret, buf);
//            send(sockfd, "OK", 2, 0);
        }
        else if(events[i].events & EPOLLPRI)
        {
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, MSG_OOB);
            if(ret <= 0)
            {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of oob content: %s\n", ret, buf);
//            send(sockfd, "OK", 2, MSG_OOB);
        }
        else
        {
            printf("something else happend\n");
        }
    }
}

void et(epoll_event *events, int number, int epollfd, int listenfd)
{
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i)
    {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd)
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_addrlen = sizeof(cli_addr);
            int connfd = accept(listenfd, (sockaddr *)&cli_addr, &cli_addrlen);
            addfd(epollfd, connfd, true);
        }
        else if(events[i].events & EPOLLIN)
        {
            printf("event trigger once\n");
            while(1)
            {
                memset(buf, 0, BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if(ret < 0)
                {
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0)
                {
                    close(sockfd);
                }
                else
                {
                    printf("get %d bytes of content: %s\n", ret, buf);
                }
            }
        }
        else if(events[i].events & EPOLLPRI)
        {
            printf("event trigger once\n");
            while(1)
            {
                memset(buf, 0, BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, MSG_OOB);
                if(ret < 0)
                {
                    if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                    {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                }
                else if(ret == 0)
                {
                    close(sockfd);
                }
                else
                {
                    printf("get %d bytes of content: %s\n", ret, buf);
                }
            }
        }
        else
        {
            printf("something else happened\n");
        }
    }

}

int main()
//int test_epoll()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = INADDR_ANY;
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(int));
    assert(listenfd >= 0);
    int ret = bind(listenfd, (sockaddr *)&addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

//    char recvbuf[1024];
//    char sendbuf[1024];
    epoll_event events[MAX_EVENT_NUMBER];
    int epfd = epoll_create(5);
    assert(epfd != -1);
    addfd(epfd, listenfd, false);
    while(1)
    {
//        memset(recvbuf, '\0', sizeof(recvbuf));
//        memset(sendbuf, '\0', sizeof(sendbuf));
        ret = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0)
        {
            printf("epoll failure\n");
            break;
        }
//        lt(events,ret,epfd,listenfd);
        et(events, ret, epfd, listenfd);
    }
    close(listenfd);
    return 0;
}
