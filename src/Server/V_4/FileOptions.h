#ifndef __FILE_OPTIONS_H_
#define __FILE_OPTIONS_H_
#include "SocketEpoll.h"

class File_Opt : public Socket {
 public:
  // friend class Socket;
 public:
  //设置读回调函数
  friend void File_Recv_Print(int, int, void *, ...);
  //设置写回调函数
  friend void File_Send_Print(int, int, void *, ...);
  //设置读回调函数【主要是处理监听套接字读事件发生】
  friend void Socket_Print(int, int, void *);
  //处理非活动链接
  friend void Do_Link(client_structure *, ...);

 public:
  File_Opt() {}
  File_Opt(int _efd, int port, char *ip) {}

  //处理文件打开请求
  void Open_recv(int sockfd, struct basic *bufp);
  //处理文件关闭请求
  void Close_recv(int sockfd, struct basic *bufp);
  //文件打开请求响应函数
  void Open_send(int sockfd, struct basic *bufp);
  //文件关闭请求响应函数
  void Close_send(int sockfd, struct basic *bufp);
  //读回调函数
  void recvdata(int sockfd, int events, void *arg);
  //写回调函数
  void senddata(int sockfd, int events, void *arg);
  //处理心跳包
  void Alive_Do(int sockfd);
  //处理非活动链接
  void Do_Inactivelink(client_structure *user_data, int efd, struct basic *ev);
  ~File_Opt() {}
};

#endif