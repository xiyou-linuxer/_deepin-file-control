#ifndef __FILE_OPTIONS_H_
#define __FILE_OPTIONS_H_
#include "model.h"
//包裹函数
struct packet {
  // hookpid
  int hookPid;
  //数据包请求类型
  int Rtype;
  //保存Mac地址
  char Amac[128];
  //路径名
  char pathname[PATH_MAX];
  //保存文件内容
  char buf[BUFF_SIZE];
};

class File_Opt {
 public:
  File_Opt() {}
  File_Opt(int _sockfd, struct packet _bufp) {
    sockfd = _sockfd;
    buf = _bufp;
  }
  void Do_File(int sockfd);
  //处理读文件请求
  void Open_Do(int sockfd, struct packet *bufp);
  //处理写文件请求
  void Close_Do(int sockfd, struct packet *bufp);
  //处理心跳包
  void Alive_Do(int sockfd);
  ~File_Opt() {}

 private:
  int sockfd;
  struct packet buf;
};

#endif