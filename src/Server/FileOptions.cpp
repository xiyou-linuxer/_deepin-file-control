#include "FileOptions.h"
#include <fstream>
#include <iostream>
#include "Timer.h"
#include "Write_Read.h"
#include "model.h"
static long num = 1;
extern int efd;
extern struct client_structure users[MAX_EVENTS];
extern struct basic _basic[MAX_EVENTS + 1];
extern Time_Heap t;
//处理回调函数
void File_Recv_Print(int sockfd, int events, void *arg, File_Opt &f) {
  f.recvdata(sockfd, events, arg);
}
void File_Send_Print(int sockfd, int events, void *arg, File_Opt &f) {
  f.senddata(sockfd, events, arg);
}
//处理文件操作
void File_Opt::recvdata(int sockfd, int events, void *arg) {
  struct basic *ev = (struct basic *)arg;
  int n = 0;
  Heap_Timer *timer = users[sockfd].timer;
  time_t cur_time;
  memset(&ev->bufp, 0, sizeof(ev->bufp));
  int sn = readn(sockfd, &ev->bufp, sizeof(ev->bufp));
  if (ev->bufp.Rtype == CLOSE) eventdelfd(efd, ev);
  if (sn > 0) {
    switch (ev->bufp.Rtype) {
      case OPEN:
        Open_recv(sockfd, ev);
        break;
      case CLOSE:
        eventset(ev, sockfd, File_Send_Print, ev);
        eventaddfd(efd, EPOLLOUT | EPOLLONESHOT | EPOLLET, ev);
        cur_time = ev->last_active;
        timer->expire = cur_time + Time_interval;
        break;
      case ALIVE:
        eventdelfd(efd, ev);
        eventset(ev, sockfd, File_Recv_Print, ev);
        eventaddfd(efd, EPOLLOUT | EPOLLONESHOT | EPOLLET, ev);
        cur_time = ev->last_active;
        timer->expire = cur_time + Time_interval;
        break;
      default:
        cout << "Invaild Package" << endl;
        break;
    }
  } else if (sn == 0) {
    cout << "client is over" << endl;
    eventdelfd(efd, ev);
    close(ev->fd);
    if (timer) t.del_timer(timer);
  } else {
    //重置选项
    if (errno == EAGAIN) {
      eventdelfd(efd, ev);
      eventset(ev, ev->fd, File_Recv_Print, ev);
      eventaddfd(efd, EPOLLOPTION, ev);
      cur_time = ev->last_active;
      timer->expire = cur_time + Time_interval;

    } else {
      //发生读错误，关闭连接并移除相应的定时器
      eventdelfd(efd, ev);
      close(ev->fd);
      if (timer) t.del_timer(timer);
    }
  }
  return;
}
//处理读操作

void File_Opt::Open_recv(int sockfd, struct basic *bufp) {
  map<string, unsigned int> file;
  Heap_Timer *timer = users[sockfd].timer;
  int fd;
  char buf[2048] = {0};
  int n = 0;
  int len = strlen(bufp->bufp.Amac);
  bufp->bufp.Amac[len - 1] = '\0';
  sprintf(buf, "%s+%s", bufp->bufp.Amac, bufp->bufp.pathname);

  //读取本地别名文件
  ifstream ins("mapdd.txt", ios::in | ios::out);
  while (!ins.eof()) {
    string key;
    unsigned value;
    ins >> key >> value;
    if (!key.length()) continue;
    file.insert(make_pair(key, value));
  }
  ins.close();
  map<string, unsigned int>::iterator itret = file.find(buf);

  //判断之前是否被备份过
  if (itret == file.end()) {
    auto ret = file.insert(make_pair(buf, num));
    char name[1024] = {0};
    sprintf(name, "%x", (unsigned int)num);
    char cwd[1024] = {0};
    if ((fd = open(name, O_CREAT | O_RDWR, 0666)) < 0) ERR_EXIT("open err");
    int c = writen(fd, bufp->bufp.buf, bufp->bufp.right - bufp->bufp.left);
    int rdret;
    int count = 0;
    while ((rdret = readn(sockfd, &bufp->bufp, sizeof(struct packet))) > 0) {
      if (rdret == -1) {
        cout << "readn 2 err" << endl;
        return;
      }
      if (!strcmp(bufp->bufp.buf, "EOF")) {
        break;
      }
      int len = bufp->bufp.right - bufp->bufp.left;
      lseek(fd, bufp->bufp.left, SEEK_SET);
      int wret = writen(fd, bufp->bufp.buf, len);
      count += wret;
    }
    close(fd);
    //从epoll删除套接字，并将套接字置为可读
    eventdelfd(efd, bufp);
    eventset(bufp, sockfd, File_Send_Print, bufp);
    eventaddfd(efd, EPOLLOUT | EPOLLET | EPOLLONESHOT, bufp);
    time_t cur_time = bufp->last_active;
    timer->expire = cur_time + Time_interval;
    char maptxt[1024];
    ofstream wr("mapdd.txt", ios::app | ios::in | ios::out);
    sprintf(maptxt, "%s %ld", buf, num);
    wr << maptxt << "\n";
    wr.close();
    cout << "接收文件完毕" << endl;
    num++;
  } else {
    cout << "文件名存在,发送it's a secret" << endl;
    char secret[] = "It's a sercet!";
    strcpy(bufp->bufp.buf, secret);
    if (writen(sockfd, &bufp->bufp, sizeof(struct packet)) < 0)
      ERR_EXIT("write server");
  }
}

//处理关闭请求
void File_Opt::Close_send(int sockfd, struct basic *bufp) {
  Heap_Timer *timer = users[sockfd].timer;
  char buf[2048] = {0};
  int len = strlen(bufp->bufp.Amac);
  bufp->bufp.Amac[len - 1] = '\0';
  sprintf(buf, "%s+%s", bufp->bufp.Amac, bufp->bufp.pathname);
  map<string, unsigned int> file;

  //读取本地存放文件别名的文件
  ifstream ins("mapdd.txt");
  while (!ins.eof()) {
    string key;
    unsigned int value;
    ins >> key >> value;
    if (!key.length()) continue;
    cout << "key" << key << "value" << value << endl;
    file.insert(make_pair(key, value));
  }
  ins.close();

  //根据key值查找文件别名并打开相应的文件
  auto ret = file.find(buf);
  if (ret != file.end()) {
    char name[1024] = {0};
    sprintf(name, "%x", ret->second);
    cout << "name close: " << name << endl;
    int fd = open(name, O_RDONLY);
    if (fd < 0) ERR_EXIT("open");
    int n;
    int len = 0;
    while ((n = read(fd, bufp->bufp.buf, sizeof(bufp->bufp.buf))) > 0) {
      if (n > 0) {
        bufp->bufp.left = len;

        len += n;
        bufp->bufp.right = len;
        int wret = writen(sockfd, &bufp->bufp, sizeof(bufp->bufp));
      }
      bufp->bufp.Rtype = CLOSE;
    }
    close(fd);
    //发送结束符
    memset(&bufp->bufp.buf, 0, sizeof(bufp->bufp.buf));
    strcpy(bufp->bufp.buf, "EOF");
    len = writen(sockfd, &bufp->bufp, sizeof(struct packet));
    if (len > 0) {
      //从epoll树结构上删除sockfd，并将桃姐子置位可写加入epoll结构
      eventdelfd(efd, bufp);
      eventset(bufp, sockfd, File_Recv_Print, bufp);
      eventaddfd(efd, EPOLLOPTION, bufp);
      time_t cur_time = bufp->last_active;
      timer->expire = cur_time + Time_interval;
    } else {
      close(bufp->fd);
      eventdelfd(efd, bufp);
    }

    //删除文件
    char namebuf[1024] = {0};
    sprintf(namebuf, "%x", ret->second);
    if (remove(namebuf) < 0) {
      cout << "remove err" << endl;
      exit(0);
    }

    //调整本地别名文件
    ofstream wr("mapdd.txt", ios::in | ios::out | ios::trunc);
    file.erase(ret);
    map<string, unsigned int>::iterator iwr = file.begin();
    while (iwr != file.end()) {
      if (!iwr->first.length()) continue;
      wr << iwr->first << " " << iwr->second << "\n";
      iwr++;
    }
    wr.close();
  }
}
void File_Opt::Open_send(int sockfd, struct basic *bufp) {
  Heap_Timer *timer = users[sockfd].timer;
  char secret[] = "It's a sercet!";
  strcpy(bufp->bufp.buf, secret);
  int wdret;

  wdret = writen(sockfd, &bufp->bufp, sizeof(struct packet));
  if (wdret == -1) {
    cout << "write err: " << __func__ << "errno :" << strerror(errno) << endl;
    close(bufp->fd);
    eventdelfd(efd, bufp);
    if (timer) t.del_timer(timer);
    return;
  }

  eventdelfd(efd, bufp);
  eventset(bufp, sockfd, File_Recv_Print, bufp);
  eventaddfd(efd, EPOLLOPTION, bufp);
  time_t cur_time = bufp->last_active;
  timer->expire = cur_time + Time_interval;
}
void File_Opt::senddata(int fd, int events, void *arg) {
  struct basic *ev = (struct basic *)arg;
  Heap_Timer *timer = users[fd].timer;
  switch (ev->bufp.Rtype) {
    case OPEN:
      Open_send(fd, ev);
      break;
    case CLOSE:
      Close_send(fd, ev);
      break;
    default:
      cout << "Invaild Err send" << endl;
      break;
  }
}
//处理定时事件
void File_Opt::Do_Inactivelink(client_structure *user_data, int efd,
                               struct basic *ev) {
  eventdelfd(efd, ev);
  close(ev->fd);
  cout << "超过规定时间，已断开连接： " << ev->fd << endl;
}
