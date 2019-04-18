//
// Created by duoml on 19-4-12.
//
#include "include.h"
#include <sys/time.h>
int testtime()
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    time_t tim = time(NULL);
    struct tm *timeinfo;
    timeinfo = localtime(&tim);
    cout << asctime(timeinfo);

}
//int main()
int test1()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = INADDR_ANY;
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >=0);
    int ret = bind(listenfd,(sockaddr*)&addr,sizeof(addr));
    assert(ret!=-1);
    ret = listen(listenfd,5);
    assert(ret != -1);
    struct sockaddr_in clientaddr;
    socklen_t cli_addrlen = sizeof(clientaddr);
    int connfd = accept(listenfd,(sockaddr*)&clientaddr,&cli_addrlen);

    char recvbuf[1024];
    char sendbuf[1024];
    fd_set read_fds;
    fd_set exception_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);
    while(1)
    {
        memset(recvbuf,'\0',sizeof(recvbuf));
        memset(sendbuf,'\0',sizeof(sendbuf));
        FD_SET(connfd,&read_fds);
        FD_SET(connfd,&exception_fds);
        ret = select(connfd+1,&read_fds,NULL,&exception_fds,NULL);
        if(ret<0)
        {
            printf("selection failure\n");
            break;
        }
        if(FD_ISSET(connfd,&read_fds))
        {
            ret =recv(connfd,recvbuf,sizeof(recvbuf)-1,0);
            if(ret<=0)
            {
                break;
            }
            printf("get %d bytes of normal data: %s\n",ret, recvbuf);
            strcpy(sendbuf,"OK");
            ret = send(connfd,sendbuf,strlen(sendbuf),0);
            if(ret<=0)
            {
                break;
            }
        }
        else if(FD_ISSET(connfd,&exception_fds))
        {
            ret = recv (connfd,recvbuf,sizeof(recvbuf)-1,MSG_OOB);
            if(ret<=0)
            {
                break;
            }
            printf("get %d bytes of oob data: %s\n",ret, recvbuf);
            strcpy(sendbuf,"OK");
            ret = send(connfd,sendbuf,strlen(sendbuf),MSG_OOB);
            if(ret<=0)
            {
                break;
            }
        }
    }
    close(connfd);
    close(listenfd);
    return 0;

}
