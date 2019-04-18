//
// Created by duoml on 19-4-12.
//

#include "include.h"
int main()
//int test()
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(2222);
    server_addr.sin_family = AF_INET;

    int ret = connect(sfd, (sockaddr *)&server_addr, sizeof(server_addr));
    if(ret == -1)
    {
        perror("connect");
        return -1;
    }
    char sendbuf[1024] = {0};
    char readbuf[1024] = {0};
    strcpy(sendbuf, "hello how are you");
    send(sfd, sendbuf, strlen(sendbuf), 0);
//    recv(sfd,readbuf,sizeof(readbuf),0);
//    printf("%s\n",readbuf);
    sleep(5);
    bzero(sendbuf, sizeof(sendbuf));
    bzero(readbuf, sizeof(readbuf));
    strcpy(sendbuf, "emergency you must handle it first");
    send(sfd, sendbuf, strlen(sendbuf), 0);
//    recv(sfd,readbuf,sizeof(readbuf),MSG_OOB);
//    printf("%s\n",readbuf);

    shutdown(sfd, SHUT_RDWR);
}
