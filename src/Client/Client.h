#pragma once
#include<limits.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include<sys/msg.h>
#include<mutex>
#include<sys/ipc.h>
#include<errno.h>
#include<thread>
#include<signal.h>
#include"MsgQueue.h"

//客户端默认接收消息为1的消息
#define MSG_TYPE 1 
//打开文件请求
#define OPEN 1
//关闭文件请求
#define CLOSE 2
//发送文件成功
#define FINISH 3
//判断是否还连接
#define ALIVE 4
//不是监控目录
#define INVAILD 5
//创建文件失败或者其他失败,一些糟糕的事情
#define FAIL 6 
//客户端和hook之间通信
#define MAC_LEN 128
//数据缓冲区的长度
#define BUF_SIZE 1024

//客户端与服务器之间的消息数据
struct Data {

    //存hookId
    int hookPid ;
    //客户端请求类型
    int type ;
    //客户端主机的mac地址
    char mac[MAC_LEN] ;
    //文件路径名
    char pathName[PATH_MAX] ;
    //数据缓冲区
    char buf[BUF_SIZE] ;
} ;

//检测心跳静态变量
namespace Keep_Alive {
static int isAlive = 0;
} ;

namespace MsgId {
static int msgId = -1 ;
} ;

namespace ServSock {
    static int servsock = 0 ;
} ;

//处理业务的入口
int ProcessHandle(char** argv) ;
//连接服务器
int Connect(const char* ip, const int port) ;
//验证是否为监控的目录
int IsMonitorDir(const char*hookPath, const char* monitorPath) ;
//向服务端发送请求
void SendData(Msg& msg, const char*monitorPath, int servFd, int msgId) ;
//接收服务端请求
int RecvData(int recvFd, int msgId) ;
//给服务器发送文件内容
int SendFile(Msg msg, int servFd) ;
//给hook发送消息
int SendHookMsg(struct Data data, int msgId, int& fd) ;
//回复文件内容
int RecoverRequest(int servFd, Msg msg) ;
//恢复文件内容
int RecoverFile(struct Data data, int& fd) ;
//判断问价是否文系统已经存在的文件
//要是系统存在的文件，返回，不是的话，
//根据hook所给的权限进行创建文件，创建成功返回1，继续下面判断文件是否
//是否时间空目下的文件；创建不成功就将文件的错误发回给hook
int IsExist(Msg msg, int msgId) ;
//处理中断信号
void SigHandle(int signo) ;
//获取文件描述符
int GetFileFd(Data data) ;
//记录日志
void printError(const char* info, size_t line) ;

