#include "SocketEpoll.h"
#include "FileOptions.h"
#include "model.h"

//自定义事件结构体
struct client_structure users[MAX_EVENTS];
//自定义链接结构体
struct basic _basic[MAX_EVENTS + 1];
//创建定时器类
Time_Heap t(MAX_EVENTS);
//设置epoll根
int efd;

extern void File_Recv_Print(int, int, void *, File_Opt &);

void Socket_Print(int lfd, int events, void *arg, File_Opt &s) {
  s.create_connfd(lfd, events, arg);
}
void Do_Link(client_structure *users, int efd, struct basic *b, File_Opt &f) {
  f.Do_Inactivelink(users, efd, b);
}
/**
 * 创建读写套接字
 */
void Socket::create_connfd(int lfd, int events, void *arg) {
  struct sockaddr_in cli;
  socklen_t len = sizeof(cli);
  int connfd;
  if ((connfd = accept(lfd, (struct sockaddr *)&cli, &len)) < 0) {
    if (errno != EAGAIN && errno != EINTR) ERR_EXIT("accept");
    cout << "func: " << __func__ << "accept: " << strerror(errno) << endl;
  }
  int i;
  do {
    //遍历全局数组，找到已经挂载到epoll树中的文件描述符
    for (i = 0; i < MAX_EVENTS; i++)
      if (_basic[i].status == 0) break;
    if (i == MAX_EVENTS) {
      cout << "max events limit: " << MAX_EVENTS << endl;
      break;
    }
    if (set_noblock(connfd) < 0) {
      cout << "fcntl failed: " << strerror(errno) << endl;
    }
    eventset(&_basic[i], connfd, File_Recv_Print, &_basic[i]);
    eventaddfd(efd, EPOLLOPTION, &_basic[i]);
    //创建定时器并绑定客户端数据
    users[connfd].address = cli;
    users[connfd].sockfd = connfd;
    auto timer = make_shared<Heap_Timer>(Time_interval);
    timer->user_data = &users[connfd];
    timer->call_back = Do_Link;
    time_t cur_time = _basic[i].last_active;
    users[connfd].timer = timer.get();
    t.add_timer(timer.get());
    cout << "Original: " << timer->expire << endl;

  } while (0);

  char buf[1024] = {0};
  cout << "connected: " << inet_ntop(AF_INET, &cli.sin_addr, buf, sizeof(buf))
       << endl;
  return;
}
/**
 * 创建监听套接字
 */
void Socket::create_listenfd(int efd, int port, const char *ip) {
  struct sockaddr_in serv;
  int listenfd;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) ERR_EXIT("socket");
  //设置非阻塞
  set_noblock(listenfd);
  eventset(&_basic[MAX_EVENTS], listenfd, Socket_Print, &_basic[MAX_EVENTS]);
  eventaddfd(efd, EPOLLIN | EPOLLET, &_basic[MAX_EVENTS]);
  int flags = 1;
  socklen_t len = sizeof(flags);

  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flags, len) < 0)
    ERR_EXIT("setsockopt");

  memset(&serv, 0, sizeof(serv));
  serv.sin_family = PF_INET;
  serv.sin_port = htons(port);
  serv.sin_addr.s_addr = INADDR_ANY;
  if (bind(listenfd, (struct sockaddr *)&serv, sizeof(serv)) < 0)
    ERR_EXIT("bind");

  if (listen(listenfd, SOMAXCONN) < 0) ERR_EXIT("listen");
  return;
}

/**
 * 设置非阻塞
 */
int Socket::set_noblock(int fd) {
  int flags;
  int ret;
  flags = fcntl(fd, F_GETFL);
  if (flags < 0) return ret;
  flags |= O_NONBLOCK;
  ret = fcntl(fd, F_SETFL, flags);
  if (ret < 0) return ret;
  return 0;
}
/***
 * 向epoll树结构中添加文件描述符及初始化相应的结构体
 */
void Epoll::eventaddfd(int efd, int events, struct basic *ev) {
  struct epoll_event pev = {0, {0}};
  int op;
  pev.data.ptr = ev;
  pev.events = ev->events = events;
  if (ev->status == 1)
    op = EPOLL_CTL_MOD;
  else {
    op = EPOLL_CTL_ADD;
    ev->status = 1;
  }
  if (epoll_ctl(efd, op, ev->fd, &pev) < 0) {
    cout << "epoll_ctl err: " << __func__ << "errno: " << strerror(errno)
         << endl;
  }
  return;
}
/***
 * 从epoll树结构中删除文件描述符
 */
void Epoll::eventdelfd(int efd, struct basic *ev) {
  struct epoll_event pev = {0, {0}};
  if (ev->status != 1) return;
  pev.data.ptr = ev;
  ev->status = 0;
  if (epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &pev) < 0) {
    cout << "epoll_ctldel err: " << __func__ << "errno: " << strerror(errno)
         << endl;
  }
  return;
}
