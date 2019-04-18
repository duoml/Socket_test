//
// Created by duoml on 19-4-17.
//

#ifndef SOCKET_TEST_TIMER_TIME_WHEEL_TIMER_H
#define SOCKET_TEST_TIMER_TIME_WHEEL_TIMER_H
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFERSIZE 64
class tw_timer;
struct client_data
{
  sockaddr_in address;
  int sockfd;
  char buf[BUFFERSIZE];
  tw_timer *timer;
};
//定时器类
class tw_timer
{
public:
  tw_timer(int rot, int ts)
      : next(NULL)
        , prev(NULL)
        , rotation(rot)
        , time_slot(ts) { }
public:
  int rotation; /*记录多少圈后定时器生效*/
  int time_slot;//记录定时器属于时间轮上哪个槽
  void (*cb_func)(client_data *); // 定时器回调函数
  client_data *user_data; //客户数据
  tw_timer *next;
  tw_timer *prev;
};

class time_wheel
{
public:
  time_wheel()
      : cur_slot(0)
  {
      for(int i = 0; i < N; ++i)
      {
          slots[i] = NULL;
      }
  }
  ~time_wheel()
  {
      for(int i = 0; i < N; ++i)
      {
          tw_timer *tmp = slots[i];
          while(tmp)
          {
              slots[i] = tmp->next;
              delete tmp;
              tmp = slots[i];
          }
      }
  }
  tw_timer *add_timer(int timeout)
  {
      if(timeout < 0)
      {
          return NULL;
      }
      int ticks = 0;
      if(timeout < SI)
      {
          ticks = 1;
      }
      else
          ticks = timeout/SI;
      int rotation = ticks/N;
      int ts = (cur_slot + ticks%N)%N;
      tw_timer *timer = new tw_timer(rotation, ts);
      if(!slots[ts])
      {
          printf("add timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot);
          slots[ts] = timer;
      }
      else
      {
          timer->next = slots[ts];
          slots[ts]->prev = timer;
          slots[ts] = timer;
      }
      return timer;
  }
  void del_timer(tw_timer *timer)
  {
      if(!timer)
      {
          return;
      }
      int ts = timer->time_slot;
      if(timer == slots[ts])
      {
          slots[ts] = slots[ts]->next;
          if(slots[ts])
          {
              slots[ts]->prev = NULL;
          }
          delete timer;
      }
      else
      {
          timer->prev->next = timer->next;
          if(timer->next)
              timer->next->prev = timer->prev;
          delete timer;
      }
  }
  void tick()
  {
      tw_timer *tmp = slots[cur_slot];
      printf("current slot is %d\n", cur_slot);
      while(tmp)
      {
          printf("tick the timer once\n");
          if(tmp->rotation > 0)
          {
              --tmp->rotation;
              tmp = tmp->next;
          }
          else
          {
              tmp->cb_func(tmp->user_data);
              if(tmp == slots[cur_slot])
              {
                  printf("delete header in cur_slot\n");
                  slots[cur_slot] = tmp->next;
                  delete tmp;
                  if(slots[cur_slot])
                  {
                      slots[cur_slot]->prev = NULL;
                  }
                  tmp = slots[cur_slot];
              }
              else
              {
                  tmp->prev->next = tmp->next;
                  if(tmp->next)
                      tmp->next->prev = tmp->prev;
                  tw_timer *tmp2 = tmp->next;
                  delete tmp;
                  tmp = tmp2;
              }
          }
      }
      cur_slot = ++cur_slot%N;

  }
private:
  static const int N = 60; //槽的数目
  static const int SI = 1; //槽间隔
  tw_timer *slots[N]; //槽 每个元素指向一个无序定时器链表
  int cur_slot; //当前槽
};
#endif //SOCKET_TEST_TIMER_TIME_WHEEL_TIMER_H
