#include "FileOptions.h"
#include "SocketEpoll.h"
#include "Write_Read.h"
#include "threadpool.cpp"
#define LENGTH 1024
extern int efd;
int main(int argc,char *argv[]) {
  int lfd, cfd;
  if (argc < 2) 
    cout << "please enter like [./a.out] [Port]" << endl;
  efd = epoll_create(MAX_EVENTS + 1);
  
  //创建线程池
  ThreadPool<struct basic > *pool = NULL;
  try {
    pool = new ThreadPool<struct basic>;
  } catch (...) {
    return 1;
  }
  Socket s;
  s.create_listenfd(efd, atoi(argv[1]), argv[2]);
   struct epoll_event events[MAX_EVENTS + 1];
  while (1) {
     int ret = epoll_wait(efd, events, MAX_EVENTS + 1, 1000);
    if (ret < 0) ERR_EXIT("epoll_wait");
     File_Opt f(efd,atoi(argv[1]),argv[2]);
    for (int i = 0; i < ret; i++) {
        struct basic *ev = (struct basic *)events[i].data.ptr;
        if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
          // ev->call_back(ev->fd,events[i].events,ev->arg,f);
          pool->append(ev,ev->fd,events[i].events,ev->arg,f);
        if((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
          pool->append(ev,ev->fd,events[i].events,ev->arg,f);
      }
    }
  return 0;
}