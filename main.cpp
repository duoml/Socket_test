#include "include.h"
static bool stop = false;
static void handle_term(int sig)
{
    stop = true;
}
int main(int argc,char* argv[])
{
    signal(SIGTERM,handle_term);

    if(argc<3)
    {
        printf("usage: %s ip_address port_number backlog\n",basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    int backlog = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock>=0);

    int reuse;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int ret = bind(sock,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret = listen(sock,backlog);
    assert(ret!=-1);
    while(!stop)
    {
        sleep(1);
    }
    close(sock);
    return 0;
}