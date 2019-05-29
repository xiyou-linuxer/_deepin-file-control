#pragma once
#include<sys/ipc.h>
#include<sys/msg.h>
#include<limits.h>
#include<stdio.h>
#include<stdlib.h>
#define MAC_LEN 128
#define MSG_FILE "/tmp"
#define MSG_ID 0x123

//消息内容
struct Packet {

    //消息请求类型
    int type;
    //客户端pid
    pid_t pid ;
    //对文件的操作
    int flag ;
    //操作权限
    mode_t mode ;
    //文件路径
    char pathName[PATH_MAX] ;
};
//消息队列格式的包
struct Msg {
    //消息类型
    long type ;
    //消息数据
    Packet buf ;
} ;  

int IpcMsgCreate() ;
int IpcMsgOpen() ;
int IpcMsgSend(int msgid, Msg& data) ;
int IpcMsgRecv(int msgid, Msg& data) ;
int IpcFree(int msgid) ;
