//
// Created by duoml on 19-4-12.
//
#include "include.h"
//int main()
int test()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.s_addr = INADDR_ANY;
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int reuse = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    assert(listenfd >= 0);
    int ret = bind(listenfd, (sockaddr *)&addr, sizeof(addr));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

//    char recvbuf[1024];
//    char sendbuf[1024];
    int connfd = accept(listenfd, NULL, NULL);
    int old_opt = fcntl(connfd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(connfd, F_SETFL, new_opt);
    char buf[1024] = {0};
    while(1)
    {
//        memset(recvbuf, '\0', sizeof(recvbuf));
//        memset(sendbuf, '\0', sizeof(sendbuf));
        bzero(buf, sizeof(buf));
        ret = recv(connfd,buf,sizeof(buf)-1,0);
        if(ret < 0)
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                printf("read later\n");
                break;
            }
            close(connfd);
            break;
        }
        else if(ret == 0)
        {
            close(connfd);
        }
        else
        {
            printf("get %d bytes of content: %s\n", ret, buf);
        }
//        lt(events,ret,epfd,listenfd);
    }
    close(connfd);
    close(listenfd);
    return 0;

}
