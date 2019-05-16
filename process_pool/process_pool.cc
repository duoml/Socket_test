//
// Created by duoml on 19-4-27.
//
#include "process_pool.h"

class cgi_conn
{
public:
  cgi_conn() { }
  ~cgi_conn() { }

  void init(int epollfd, int sockfd, const sockaddr_in &client_addr)
  {
      m_epollfd = epollfd;
      m_sockfd = sockfd;
      m_address = client_addr;
      bzero(m_buf, BUFFER_SZIE);
      m_read_idx = 0;
  }

  void process()
  {
      int idx = 0;
      int ret = -1;
      while(true)
      {
          idx = m_read_idx;
          ret = recv(m_sockfd, m_buf + idx, BUFFER_SZIE - 1, 0);
          if(ret < 0)
          {
              if(errno != EAGAIN)
              {
                  removefd(m_epollfd, m_sockfd);
              }
              break;
          }
          else if(ret == 0)
          {
              removefd(m_epollfd, m_sockfd);
              break;
          }
          else
          {
              m_read_idx += ret;
              printf("user content is: %s\n", m_buf);
              for(; idx < m_read_idx; ++idx);
              {
                  if((idx >= 1) && (m_buf[idx - 1] == '\r') && (m_buf[idx] == '\n'))
                  {
                      break;
                  }
              }
              if(idx == m_read_idx)
              {
                  continue;
              }
              m_buf[idx - 1] = '\0';
              char *file_name = m_buf;
              if(access(file_name, F_OK) == -1)
              {
                  removefd(m_epollfd, m_sockfd);
                  break;
              }
              ret = fork();
              if(ret == -1)
              {
                  removefd(m_epollfd, m_sockfd);
                  break;
              }
              else if(ret > 0)
              {
                  removefd(m_epollfd, m_sockfd);
                  break;
              }
              else
              {
                  close(STDOUT_FILENO);
                  dup(m_sockfd);
                  execl(m_buf, m_buf, 0);
                  exit(0);
              }
          }
      }
  }
private:
  static const int BUFFER_SZIE = 1024;
  static int m_epollfd;
  int m_sockfd;
  sockaddr_in m_address;
  char m_buf[BUFFER_SZIE];
  int m_read_idx;
};

int cgi_conn::m_epollfd = -1;

int main()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd > 0);

    int ret = 0;
    sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_port = htons(3333);
    address.sin_addr.s_addr = INADDR_ANY;
//    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr);
    address.sin_family = AF_INET;

    int set=1;
    ret = setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&set,sizeof(set));
    assert(ret!=-1);

    ret = bind(listenfd, (sockaddr *)&address, sizeof(address));
    if(ret ==-1)
    {
        perror("bind");
    }
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    processpool<cgi_conn>* pool =  processpool<cgi_conn>::create(listenfd);
    if(pool)
    {
        pool->run();
        delete pool;
    }
    close(listenfd);
    return 0;
}