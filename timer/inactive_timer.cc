//
// Created by duoml on 19-4-16.
//
//#include "list_timer.h"
#include "include.h"
//#include "time_wheel_timer.h"
#include "min_heap_timer.h"
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>
#define FD_LIMIT 65535
#define MAX_EVENT_NUMBER 1024
#define TIMESLOT 1
#define DEFAULT_TIME 5

static int pipefd[2];
//static sort_timer_lst timer_list;
//static time_wheel timer_tw;
static time_heap h_timer(20);
static int epollfd = 0;
int setnonblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}
void addfd(int epollfd, int fd)
{
    epoll_event event;
    bzero(&event, sizeof(event));
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig)
{
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

void timer_handler()
{
//    timer_list.tick();
//    timer_tw.tick();
    h_timer.tick();
    int timeslot = 1;
    if(!h_timer.empty())
        timeslot = h_timer.top()->expire-time(NULL);
    alarm(timeslot);
}

void cb_func(client_data *user_data)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    printf("close fd %d\n", user_data->sockfd);
}
int main()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = INADDR_ANY;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int reuse=0;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    int ret = bind(listenfd, (sockaddr *)&addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addfd(epollfd, listenfd);

    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    addsig(SIGALRM);
    addsig(SIGTERM);
    bool stop_server = false;

    client_data *users = new client_data[FD_LIMIT];
    bool timeout = false;
    alarm(TIMESLOT);
    while(!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0) && (errno != EINTR))
        {
            printf("epoll failure\n");
            break;
        }
        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (sockaddr *)&client_address, &client_addrlength);
                addfd(epollfd, connfd);
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;
//list timer
//                util_timer *timer = new util_timer;
//                timer->user_data = &users[connfd];
//                timer->cb_func = cb_func;
//                time_t cur = time(NULL);
//                timer->expire = cur + 3*TIMESLOT;
//                users[connfd].timer = timer;
//                timer_list.add_timer(timer);
//time wheel timer
//                tw_timer *timer = timer_tw.add_timer(60);
//                timer->user_data = &users[connfd];
//                users[connfd].timer = timer;
//                timer->cb_func = cb_func;
//min heap timer
                heap_timer *timer = new heap_timer(3*DEFAULT_TIME);
                timer->user_data = &users[connfd];
                timer->cb_func = cb_func;
                users[connfd].timer = timer;
                h_timer.add_timer(timer);
            }
            else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
            {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if(ret == -1)
                {
                    continue;
                }
                else if(ret == 0)
                {
                    continue;
                }
                else
                {
                    for(int i = 0; i < ret; ++i)
                    {
                        switch(signals[i])
                        {
                        case SIGALRM:
                        {
                            timeout = true;
                            break;
                        }
                        case SIGTERM:stop_server = true;
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                bzero(users[sockfd].buf, BUFFER_SIZE);
                ret = recv(sockfd, users[sockfd].buf, BUFFER_SIZE- 1, 0);
                printf("get %d bytes of client data %s from %d\n", ret, users[sockfd].buf, sockfd);
//                tw_timer *timer = users[sockfd].timer;
                heap_timer *timer = users[sockfd].timer;
                if(ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        cb_func(&users[sockfd]);
                        if(timer)
                        {
//                            timer_tw.del_timer(timer);
                            h_timer.del_timer(timer);
                        }
                    }
                }
                else if(ret == 0)
                {
                    cb_func(&users[sockfd]);
                    if(timer)
                    {
                        h_timer.del_timer(timer);
//                        timer_tw.del_timer(timer);
                    }
                }
                else
                {
                    if(timer)
                    {
//                            time_t cur = time(NULL);
//                            timer->expire = cur + 3*TIMESLOT;
                        printf("adjust timer once\n");
//                            timer_list.adjust_timer(timer);
//                        timer_tw.del_timer(timer);
//                        tw_timer *timer = timer_tw.add_timer(60);
                        time_t cur = time(NULL);
                        timer->expire = cur+3*DEFAULT_TIME;
                        h_timer.percolate_down(timer->index);
                    }
                }
            }
            else
            {
            }
        }
        if(timeout)
        {
            timer_handler();
            timeout = false;
        }
    }
}
