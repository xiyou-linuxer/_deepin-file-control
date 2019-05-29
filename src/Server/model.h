#ifndef _MODEL_H
#define _MODEL_H
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>

inline void ERR_EXIT(const char *m) {
  perror(m);
  exit(EXIT_FAILURE);
}
using namespace std;
//文件请求类型
const int OPEN = 1;
const int CLOSE = 2;
const int ALIVE = 6;
const int MAX_EVENTS = 1000;
const int BUFF_SIZE = 4096;

using namespace std;
//包裹函数
struct packet {
  // hookpid
  int hookPid;
  //数据包请求类型
  int Rtype;
  long left, right;
  //保存Mac地址
  char Amac[128];
  //路径名
  char pathname[PATH_MAX];
  //保存文件内容
  char buf[BUFF_SIZE];
};
class File_Opt;
#endif
