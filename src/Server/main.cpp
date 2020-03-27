#include "FileOptions.h"
#include "SocketEpoll.h"
#include "ThreadPool.cpp"
#include "Write_Read.h"

extern Time_Heap t;
extern int efd;
extern struct client_structure users[MAX_EVENTS];
extern struct basic _basic[MAX_EVENTS + 1];

int main(int argc, char *argv[]) {
  int lfd, cfd;
  signal(SIGPIPE, SIG_IGN);
  if (argc < 2) cout << "please enter like [./a.out] [Port]" << endl;
  efd = epoll_create(MAX_EVENTS + 1);

  //创建线程池
  ThreadPool<struct basic> *pool = NULL;
  try {
    pool = new ThreadPool<struct basic>;
  } catch (...) {
    return 1;
  }
  Socket s;
  s.create_listenfd(efd, atoi(argv[1]), argv[2]);
  int i, count = 0;
  File_Opt f(efd, atoi(argv[1]), argv[2]);
  struct basic *time = NULL;
  Heap_Timer *timer = NULL;
  struct epoll_event events[MAX_EVENTS + 1];
  int pos = 0;
  count = 1;
  while (1) {
    //如果客户端只建立链接不发请求，超过规定时间服务器主动关闭链接
    for (int i = 0; i < 100; i++, pos++) {
      if (pos == MAX_EVENTS) pos = 0;
      if (_basic[pos].status != 1) continue;
      time = &_basic[pos];
      timer = users[time->fd].timer;
      if (timer) t.tick(f, time);
    }
    int ret = epoll_wait(efd, events, MAX_EVENTS + 1, 1000);
    if (ret < 0) ERR_EXIT("epoll_wait");
    for (int i = 0; i < ret; i++) {
      struct basic *ev = (struct basic *)events[i].data.ptr;

      if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
        pool->append(ev, ev->fd, events[i].events, ev->arg, f);
      if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
        pool->append(ev, ev->fd, events[i].events, ev->arg, f);
    }
  }
  return 0;
}