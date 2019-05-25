#include "FileOptions.h"
#include "SocketEpoll.h"
#include "Write_Read.h"
#include "threadpool.cpp"
#define LENGTH 1024

int main(int argc, char *argv[]) {
  int lfd, cfd;
  if (argc < 2) cout << "please enter like [./a.out] [Port]" << endl;
  int efd = epoll_create(100);
  //创建线程池
  ThreadPool<File_Opt> *pool = NULL;
  try {
    pool = new ThreadPool<File_Opt>;
  } catch (...) {
    return 1;
  }
  Socket s;
  lfd = s.create_listenfd(efd, atoi(argv[1]), argv[2]);
  struct epoll_event events[100];
  while (1) {
    int ret = epoll_wait(efd, events, 100, -1);
    if (ret < 0) ERR_EXIT("epoll_wait");
    struct fds *sfd = NULL;
    File_Opt *f;
    for (int i = 0; i < ret; i++) {
      int sockfd = events[i].data.fd;
      if (sockfd == lfd)
        s.create_connfd(efd, lfd, true);
      else if (events[i].events & EPOLLIN) {
        cout << "sockfd: " << sockfd << endl;
        pool->append(f, sockfd);
      }
    }
  }
  close(lfd);
  return 0;
}