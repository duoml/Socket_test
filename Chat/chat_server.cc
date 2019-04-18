//
// Created by duoml on 19-4-15.
//

#include "include.h"
#include <poll.h>
#define USER_LIMIT 5
#define BUFFER_SIZE 64
#define FD_LIMIT 65535
struct client_data
{
  sockaddr_in address;
  char *write_buf;
  char buf[BUFFER_SIZE];
};

int setnonblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;
}

int main()
{
    int ret = 0;
    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(2222);
    address.sin_addr.s_addr = INADDR_ANY;
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd > 0);
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);
    client_data *users = new client_data[FD_LIMIT];
    pollfd fds[USER_LIMIT];
    int user_counter = 0;
    for(int i = 1; i < USER_LIMIT; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;
    while(1)
    {
        ret = poll(fds, USER_LIMIT + 1, -1);
        if(ret < 0)
        {
            printf("poll failure\n");
            break;
        }
        for(int i = 0; i < USER_LIMIT + 1; ++i)
        {
            if((fds[i].fd == listenfd) && (fds[i].revents & POLLIN))
            {
                struct sockaddr_in cli_addr;
                bzero(&cli_addr, sizeof(cli_addr));
                socklen_t addrlen = sizeof(cli_addr);

                int connfd = accept(listenfd, (sockaddr *)&cli_addr, &addrlen);
                if(connfd < 0)
                {
                    perror("accept error");
                    continue;
                }
                if(user_counter >= USER_LIMIT)
                {
                    const char *info = "too many user\n";
                    printf("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                user_counter++;
                users[connfd].address = address;
                setnonblocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user,now have %d users\n", user_counter);
            }
            else if(fds[i].revents & POLLERR)
            {
                printf("get an error form %d\n", fds[i].fd);
                char errors[100] = {0};
                socklen_t length = sizeof(errors);
                if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    printf("get socket option failed\n");
                }
                continue;
            }
            else if(fds[i].revents & POLLRDHUP)
            {
                users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                fds[i] = fds[user_counter];
                --i;
                --user_counter;
                printf("a client left\n");
            }
            else if(fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                bzero(users[connfd].buf, BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE - 1, 0);
                if(ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        --i;
                        --user_counter;
                    }
                }
                else if(ret == 0)
                {
                }
                else
                {
                    for(int j = 1; j < user_counter; ++j)
                    {
                        if(fds[j].fd == connfd)
                            continue;
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].buf;
                    }
                }
            }
            else if(fds[i].revents & POLLOUT)
            {
                int connfd = fds[i].fd;
                if(!users[connfd].write_buf)
                {
                    continue;
                }
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                fds[i].events |= ~POLLOUT;
                fds[i].events |= POLLIN;
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}
