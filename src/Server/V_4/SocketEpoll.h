#ifndef __SOCKET_EPOLL_H_
#define __SOCKET_EPOLL_H_
#include "Timer.h"
#include "model.h"

const unsigned int EPOLLOPTION = EPOLLONESHOT | EPOLLIN | EPOLLET;

class File_Opt;

struct basic {
 public:
  int fd;      //要监听的文件描述符
  int events;  //对应的监听事件
  void *arg;   //用来初始化回调函数的参数 //泛型参数
  void (*call_back)(int fd, int events, void *arg, File_Opt &f);  //回调函数
  int status;  //是否在监听:1->在红黑树上(监听), 0->不在(不监听)
  struct packet bufp;  //数据结构体
  int len;             //数据长度
  long last_active;    //最后的活动时间
};

class Epoll {
 public:
  Epoll() {}

 public:
  void eventset(struct basic *_basic, int _fd,
                void (*_call)(int fd, int events, void *arg, File_Opt &f),
                void *_arg) {
    _basic->fd = _fd;
    _basic->events = 0;
    _basic->arg = _arg;
    _basic->call_back = _call;
    _basic->status = 0;
    _basic->last_active = time(NULL);
  }
  //向Epoll中添加描述符
  void eventaddfd(int efd, int events, struct basic *_basic);
  //从epoll种删除描述符
  void eventdelfd(int efd, struct basic *_basic);
};

// socket函数
class Socket : public Epoll {
 public:
  friend void Socket_Print(int, int, void *, ...);

  Socket() {}

 public:
  //创建监听套接字
  void create_listenfd(int efd, int port, const char *ip);
  //创建读写套接字
  void create_connfd(int efd, int lfd, void *arg);
  //设置非阻塞
  int set_noblock(int fd);

 private:
  char *ip = NULL;
  int port;
};
#endif
