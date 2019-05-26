#ifndef __FILE_OPTIONS_H_
#define __FILE_OPTIONS_H_
#include "SocketEpoll.h"
// #include "model.h"

class File_Opt : public Socket {
 public:
  // friend class Socket;
 public:
  friend void File_Recv_Print(int, int, void *, ...);
  friend void File_Send_Print(int, int, void *, ...);
  friend void Socket_Print(int, int, void *);
  friend void Do_Link(client_structure *, ...);

 public:
  File_Opt() {}
  // File_Opt(int _efd,int port,char *ip) : Socket(_efd,port,ip) {}
  File_Opt(int _efd, int port, char *ip) {}
  void Do_File(int sockfd);
  //处理读文件请求
  void Open_recv(int sockfd, struct basic *bufp);
  //处理写文件请求
  void Close_recv(int sockfd, struct basic *bufp);
  void Open_send(int sockfd, struct basic *bufp);
  void Close_send(int sockfd, struct basic *bufp);
  void recvdata(int sockfd, int events, void *arg);
  void senddata(int sockfd, int events, void *arg);
  //处理心跳包
  void Alive_Do(int sockfd);
  //处理非活动链接
  void Do_Inactivelink(client_structure *user_data, int efd, struct basic *ev);
  ~File_Opt() {}

 private:
  // int sockfd;
};

#endif