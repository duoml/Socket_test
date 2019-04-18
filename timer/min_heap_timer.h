//
// Created by duoml on 19-4-18.
//

#ifndef SOCKET_TEST_TIMER_MIN_HEAP_TIMER_H
#define SOCKET_TEST_TIMER_MIN_HEAP_TIMER_H

#include <iostream>
#include <netinet/in.h>
#include <time.h>

using std::exception;
#define BUFFER_SIZE 64

class heap_timer;
struct client_data
{
  sockaddr_in address;
  int sockfd;
  char buf[BUFFER_SIZE];
  heap_timer *timer;
};
class heap_timer
{
public:
  heap_timer(int delay)
  {
      expire = time(NULL) + delay;
  }
public:
  time_t expire;
  void (*cb_func)(client_data *);
  client_data *user_data;
  int index;
};

class time_heap
{
public:
  time_heap(int cap) noexcept(false)
      : capacity(cap)
        , cur_size(0)
  {
      array = new heap_timer *[capacity];
      if(!array)
          throw std::exception();
      for(int i = 0; i < capacity; ++i)
          array[i] = nullptr;
  }
  time_heap(heap_timer **init_array, int size, int capacity) noexcept(false)
      : cur_size(size)
        , capacity(capacity)
  {
      if(capacity < size)
          throw std::exception();
      array = new heap_timer *[capacity];
      if(!array)
          throw std::exception();
      for(int i = 0; i < size; ++i)
      {
          array[i] = NULL;
      }
      if(size != 0)
      {
          for(int i = 0; i < size; ++i)
          {
              array[i] = init_array[i];
              array[i]->index = i;
          }
          for(int i = (cur_size - 1)/2; i >= 0; --i)
          {
              percolate_down(i);
          }
      }
  }
  ~time_heap()
  {
      for(int i = 0; i < cur_size; ++i)
      {
          delete array[i];
      }
      delete[] array;
  }
public:
  void add_timer(heap_timer *timer) noexcept(false)
  {
      if(!timer)
          return;
      if(cur_size >= capacity)
          resize();
      int hole = cur_size++;
      int parent = 0;
      for(; hole > 0; hole = parent)
      {
          parent = (hole - 1)/2;
          if(array[parent]->expire <= timer->expire)
              break;
          array[hole] = array[parent];
          array[hole]->index = hole;
      }
      array[hole] = timer;
      array[hole]->index = hole;
  }

  void del_timer(heap_timer *timer)
  {
      if(!timer)
          return;
      //延迟销毁
      timer->cb_func = NULL;
  }

  heap_timer *top() const
  {
      if(empty())
          return NULL;
      return array[0];
  }

  void pop_timer()
  {
      if(empty())
          return;
      if(array[0])
      {
          delete array[0];
          array[0] = array[--cur_size];
          array[0]->index=0;
          percolate_down(0);
      }
  }

  void tick()
  {
      printf("tick once\n");
      heap_timer *tmp = array[0];
      time_t cur = time(NULL);
      while(!empty())
      {
          if(!tmp)
              break;
          if(tmp->expire > cur)
              break;
          if(array[0]->cb_func)
              array[0]->cb_func(array[0]->user_data);
          pop_timer();
          tmp = array[0];
      }
  }

  bool empty() const { return cur_size == 0; }
//private:
  //最小堆的下虑操作
  void percolate_down(int hole)
  {
      heap_timer *tmp = array[hole];
      int child = 0;
      for(; ((hole*2 + 1) <= (cur_size - 1)); hole = child)
      {
          child = hole*2 + 1;
          if((child < (cur_size - 1)) && (array[child + 1]->expire < array[child]->expire))
              ++child;
          if(array[child]->expire < tmp->expire)
          {
              array[hole] = array[child];
              array[hole]->index = hole;
          }
          else
              break;
      }
      array[hole] = tmp;
      array[hole]->index = hole;
  }
private:
  void resize() noexcept(false)
  {
      heap_timer **tmp = new heap_timer *[2*capacity];
      for(int i = 0; i < 2*capacity; ++i)
      {
          tmp[i] = NULL;
      }
      if(!tmp)
          throw std::exception();
      capacity = 2*capacity;
      for(int i = 0; i < cur_size; ++i)
      {
          tmp[i] = array[i];
          tmp[i]->index = i;
      }
      delete[] array;
      array = tmp;
  }
  heap_timer **array;
  int capacity;
  int cur_size;
};

#endif //SOCKET_TEST_TIMER_MIN_HEAP_TIMER_H
