//
// Created by duoml on 19-4-12.
//
#include "include.h"
#define BUFFER_SIZE 1023
int setnonblocking(int fd)
{
    int old_opt = fcntl(fd,F_GETFL);
    int new_opt = old_opt|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_opt);
    return old_opt;
}
int unblock_connect(const char* ip,int port,int time)
{
    int ret =0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
//    address.sin_addr.s_addr = inet_addr(ip);
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    int fdopt = setnonblocking(sockfd);
    ret = connect(sockfd,(sockaddr*)&address,sizeof(address));
    if(ret == 0)
    {
        printf("connect with server immediately\n");
        fcntl(sockfd,F_SETFL,fdopt);
        return sockfd;
    }
    else if(errno!=EINPROGRESS)
    {
        printf("unblock connect not support");
        return -1;
    }
    fd_set readfds;
    fd_set writefds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd,&writefds);

    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    ret = select(sockfd+1,NULL,&writefds,NULL,&timeout);
    if(ret <=0)
    {
        printf("connection time out\n");
        close(sockfd);
        return -1;
    }
    if(!FD_ISSET(sockfd,&writefds))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }
    int error = 0;
    socklen_t  length = sizeof(error);
    if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&length)<0)
    {
        printf("get sock option failed\n");
        close(sockfd);
        return -1;
    }
    if(error!=0)
    {
        printf("connection failed after select with the error: %d \n",error);
        close(sockfd);
        return -1;
    }
    printf("connection ready after select with the socket: %d \n",sockfd);
    fcntl(sockfd,F_SETFL,fdopt);
    return sockfd;

}
//int main()
int test_nonblock()
{
    int sockfd = unblock_connect("127.0.0.1",2222,10);
    if(sockfd<0)
    {
        return 1;
    }
    close(sockfd);
    return 0;
}
