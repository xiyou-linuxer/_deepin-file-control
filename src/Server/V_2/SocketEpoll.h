#ifndef __SOCKET_EPOLL_H_
#define __SOCKET_EPOLL_H_
#include "model.h"
const unsigned int EPOLLOPTION = EPOLLONESHOT | EPOLLIN | EPOLLET;
class File_Opt;
typedef void (*Pcall)(int fd, int events, void *arg, File_Opt &f);
struct basic {
 public:
  int fd;      //要监听的文件描述符
  int events;  //对应的监听事件
  void *arg;
  // Pcall call_back;                                              //泛型参数
  void (*call_back)(int fd, int events, void *arg, File_Opt &f);  //回调函数
  int status;  //是否在监听:1->在红黑树上(监听), 0->不在(不监听)
  struct packet bufp;
  int len;
  long last_active;
};

class Epoll {
 public:
  Epoll() {}
  // Epoll (int _efd) :efd(_efd) {}
 public:  //设置fd状态
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

 public:
  // int efd;
  // struct basic _basic[MAX_EVENTS + 1];
};

// socket函数
class Socket : public Epoll {
 public:
  friend void Socket_Print(int, int, void *, ...);
  Socket() {}
  // Socket(int _efd) : Epoll(_efd) {}
 public:
  typedef void (Socket::*p)(int, int, void *);
  //创建监听套接字
  void create_listenfd(int efd, int port, const char *ip);
  //创建读写套接字
  void create_connfd(int efd, int lfd, void *arg);

  // void add_fd(int efd,int fd,bool enable);
  int set_noblock(int fd);
  //重置oneshot选项
  //	void reset_shot(int efd,int fd);
  // void Print(int efd,int lfd,void *arg);
 private:
  char *ip = NULL;
  int port;
  bool enable;
};
#endif
