#ifndef __WRITE_HEAD_H_
#define __WRITE_HEAD_H_
#include "model.h"

// IO函数
int readn(int fd, void* buf, size_t size);
int writen(int fd, void* buf, size_t size);

#endif