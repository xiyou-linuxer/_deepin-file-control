#ifndef __TIMER__H__
#define __TIMER__H__
#include <mutex>
#include "model.h"
using std::exception;

const int Length = 1000;
const int Time_interval = 600;
class Heap_Timer;

//客户端数据结构
struct client_structure {
  sockaddr_in address;
  int sockfd;
  char buf[Length];
  Heap_Timer *timer;

 public:
  client_structure() {}
};
class Socket;
//定时器类
class Heap_Timer {
  // friend class Socket;
 public:
  Heap_Timer(int delay) { expire = time(NULL) + delay; }

 public:
  time_t expire;
  void (*call_back)(client_structure *, int, struct basic *, File_Opt &f);
  client_structure *user_data;
};
//时间堆类
class Time_Heap {
 public:
  Time_Heap(const int cap) : capacity(cap), cur_size(0) {
    array = new Heap_Timer *[capacity];
    if (!array) {
      throw std::exception();
    }
    for (int i = 0; i < capacity; i++) {
      array[i] = NULL;
    }
  }
  Time_Heap(Heap_Timer **_array, int size, int capacity)
      : cur_size(0), capacity(capacity) {
    if (capacity < size) {
      throw std::exception();
    }
    array = new Heap_Timer *[capacity];
    if (!array) {
      throw std::exception();
    }
    for (int i = 0; i < capacity; i++) {
      array[i] = NULL;
    }
    if (size != 0) {
      for (int i = 0; i < size; i++) {
        array[i] = _array[i];
      }
      for (int i = (cur_size - 1) / 2; i >= 0; --i) {
        Heap_down(i);
      }
    }
  }
  ~Time_Heap() {
    for (int i = 0; i < Org_size; i++) {
      delete array[i];
    }
    delete[] array;
  }

 public:
  //添加定时器
  void add_timer(Heap_Timer *timer);
  //删除定时器
  void del_timer(Heap_Timer *timer);
  //获取顶部定时器
  Heap_Timer *top() const;
  //删除顶部定时器
  void pop_timer();
  //心搏函数
  void tick(File_Opt &f, struct basic *);
  //判断当前是否存在定时器
  inline bool empty() const { return cur_size == 0; }

 private:
  void Heap_down(int i);
  void Resize();

 private:
  //堆数组
  Heap_Timer **array;
  //堆数组的容量
  int capacity;
  //当前元素个数
  int cur_size;
  int Org_size;
  int Org_num = 0;
};
#endif