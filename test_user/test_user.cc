//
// Created by duoml on 19-4-9.
//
#include "include.h"
#include <sys/resource.h>
#include <sys/stat.h>
static bool switch_to_user(uid_t user_id, gid_t gp_id)
{
    if((user_id == 0) && (gp_id == 0))
    {
        return false;
    }

    gid_t gid = getgid();
    uid_t uid = getuid();
    if(((gid != 0) || (uid != 0)) && ((gid != gp_id) || (uid != user_id)))
    {
        return false;
    }
    if(uid!=0)
    {
        return false;
    }
    if((setgid(gp_id)<0)||(setuid(user_id)<0))
    {
        return false;
    }
    return true;
}

int test_limit()
{
//    struct rlimit rlim;
//    bzero(&rlim,sizeof(rlim));
//    getrlimit(RLIMIT_AS,&rlim);
//    cout << rlim.rlim_cur << endl;
//    cout << rlim.rlim_max << endl;
//    getrlimit(RLIMIT_CORE,&rlim);
//    cout << rlim.rlim_cur << endl;
//    cout << rlim.rlim_max << endl;
//    getrlimit(RLIMIT_CPU,&rlim);
//    cout << rlim.rlim_cur << endl;
//    cout << rlim.rlim_max << endl;
//    getrlimit(RLIMIT_DATA,&rlim);
//    cout << rlim.rlim_cur << endl;
//    cout << rlim.rlim_max << endl;
//    getrlimit(RLIMIT_FSIZE,&rlim);
//    cout << rlim.rlim_cur << endl;
//    cout << rlim.rlim_max << endl;

}

bool daemonize()
{
    pid_t pid = fork();
    if(pid<0)
    {
        return false;
    }
    else if(pid>0)
    {
        exit(0);
    }
    umask(0);
    pid_t sid = setsid();
    if(sid <0)
    {
        return false;
    }
    if((chdir("/"))<0)
    {
        return false;
    }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null",O_RDONLY);
    open("/dev/null",O_RDWR);
    open("/dev/null",O_RDWR);
    return true;
}
int main()
{
    daemonize();
    while(1)
    {
        sleep(2);
    }
}
