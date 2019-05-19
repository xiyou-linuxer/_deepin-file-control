#ifndef _MODEL_H
#define _MODEL_H
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <limits.h>
#include <iostream>
#include <memory>
#include <map>
#include <semaphore.h>
#include <iostream>
#define ERR_EXIT(m) \
 do { \
    perror(m); \
    exit(EXIT_FAILURE); \
}while(0)
using namespace std;
//文件请求类型
#define OPEN 1
#define CLOSE 2
#define ALIVE 3
#define SHM_SIZE 1024
#define BUFF_SIZE 1024

using namespace std;
#endif
