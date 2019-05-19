#include "FileOptions.h"
#include <fstream>
#include <iostream>
#include "Write_Read.h"
#include "model.h"
static long num = 1;

//处理文件操作
void File_Opt::Do_File(int sockfd) {
  int n = 0;
  struct packet bufp;
  memset(&bufp, 0, sizeof(bufp));
  int sn = 0;
  while ((sn = readn(sockfd, &bufp, sizeof(bufp)))) {
    if (sn == -1) {
      cout << "read 1 err" << endl;
      close(sockfd);
      return;
    }
    cout << "file_opt content: " << bufp.buf << endl;
    switch (bufp.Rtype) {
      case OPEN:
        Open_Do(sockfd, &bufp);
        break;
      case CLOSE:
        Close_Do(sockfd, &bufp);
        break;
    //   case ALIVE:
    //     Alive_Do(sockfd);
    //     break;
      default:
        cout << "Invaild Package" << endl;
        break;
    }
  }
  return;
}
//处理读操作

void File_Opt::Open_Do(int sockfd, struct packet *bufp) {
  map<string, unsigned int> file;
  int fd;
  char buf[2048] = {0};
  int n = 0;
  int len = strlen(bufp->Amac);
  bufp->Amac[len - 1] = '\0';
  sprintf(buf, "%s+%s", bufp->Amac, bufp->pathname);

  //读取本地别名文件
  ifstream ins("mapdd.txt", ios::in | ios::out);
  while (!ins.eof()) {
    string key;
    unsigned value;
    ins >> key >> value;
    file.insert(make_pair(key, value));
  }
  ins.close();
  map<string, unsigned int>::iterator itret = file.find(buf);

  //判断之前是否被备份过
  if (itret == file.end()) {
    auto ret = file.insert(make_pair(buf, num));
    char name[1024] = {0};
    sprintf(name, "%x", (unsigned int)num);
    cout << "转换成的名字：" << name << endl;
    char cwd[1024] = {0};
    if ((fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0)
      ERR_EXIT("open err");
    writen(fd, bufp->buf, strlen(bufp->buf));
    int rdret;
    while ((rdret = readn(sockfd, bufp, sizeof(struct packet))) > 0) {
      if (rdret == -1) {
        cout << "readn 2 err" << endl;
        return;
      }
      if (!strlen(bufp->buf) || !strcmp(bufp->buf, "EOF")) {
        char secret[] = "It's a sercet!";
        strcpy(bufp->buf, secret);
        int wdret;
        wdret = writen(sockfd, bufp, sizeof(struct packet));
        if (wdret == -1) {
          cout << "write err" << endl;
          return;
        }
        break;
      }
      writen(fd, bufp->buf, strlen(bufp->buf));
    }
    close(fd);
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
    strcpy(bufp->buf, secret);
    if (writen(sockfd, bufp, sizeof(struct packet)) < 0)
      ERR_EXIT("write server");
  }
}

//处理关闭请求
void File_Opt::Close_Do(int sockfd, struct packet *bufp) {
  char buf[2048] = {0};
  int len = strlen(bufp->Amac);
  bufp->Amac[len - 1] = '\0';
  sprintf(buf, "%s+%s", bufp->Amac, bufp->pathname);
  map<string, unsigned int> file;
  cout << buf << endl;

  //读取本地存放文件别名的文件
  ifstream ins("mapdd.txt");
  while (!ins.eof()) {
    string key;
    unsigned int value;
    ins >> key >> value;
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
    while ((n = readn(fd, bufp->buf, sizeof(bufp->buf))) > 0) {
      if (n != sizeof(bufp->buf)) bufp->buf[n] = 0;
      int wret = writen(sockfd, bufp, sizeof(struct packet));
      cout << "packet type: " << bufp->Rtype << endl;
      bufp->Rtype = CLOSE;
      cout << "The content That has sent" << bufp->buf << endl;
    }
    close(fd);
    // char namebuf[1024] = {0};
    // sprintf(namebuf,"%x",ret->second);
    // if(remove(namebuf) < 0) {
    //     cout<<"remove err"<<endl;
    //     exit(0);
    // }

    //调整本地别名文件
    // ofstream wr("mapdd.txt",ios::in |ios::out);
    // map <string,unsigned int> ::iterator itfile = file.begin();
    // while(itfile != file.end()) {
    //     if(strcmp(ret->first,ret->second))
    //         wr<<itfile->first<<" "<<itfile->second<<"\n";
    //     else
    //         itfile++;
    // }
    // wr.close();

    memset(bufp->buf, 0, sizeof(bufp->buf));
    strcpy(bufp->buf, "EOF");
    int ret = writen(sockfd, bufp, sizeof(struct packet));
    cout << "ret = " << ret << endl;
    if (ret < 0) {
      cout << "writen close err" << endl;
      return;
    }
    cout << "hookpid" << bufp->hookPid << endl;
  }
}