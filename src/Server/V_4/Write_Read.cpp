#include "Write_Read.h"
#include "FileOptions.h"
#include "model.h"
int count = 0;
//每次读取size大小的文件
int readn(int fd, void *buf, size_t size) {
  char *bufp = (char *)buf;
  int nleft = size;
  int nread;
  while (nleft > 0) {
    if ((nread = read(fd, bufp, nleft)) < 0) {
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
      cout << strerror(errno) << endl;
      return -1;
    }
    if (nread == 0) {
      return nread;
    }
    nleft -= nread;
    bufp += nread;
  }
  return size - nleft;
}

//每次写入size大小的文件
int writen(int fd, void *buf, size_t size) {
  int nwrite;
  char *bufp = (char *)buf;
  int nleft = size;
  while (nleft > 0) {
    if ((nwrite = write(fd, bufp, nleft)) < 0) {
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
        continue;
      }
      return -1;
    }
    if (nwrite == 0) continue;
    nleft -= nwrite;
    bufp += nwrite;
  }
  return size - nleft;
}