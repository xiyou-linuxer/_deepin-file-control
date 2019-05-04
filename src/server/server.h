#pragma once
#include<iostream>
#include<arpa/inet.h>
#include<fstream>
#include<thread>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<sys/types.h>
using namespace std ;

#define LEN 1024
#define PORT 2345 
#define IP "127.0.0.1" 
//存储文件信息的结构
typedef struct file_info {
 
    //recv返回值，暂时保留
    int cb_num ;
    //服务端套接字，用户发送消息
    int serv_fd ;
    //获取打开文件所需要的fd
    int fd ;
    //获取对文件的操作
    int flag ;/*
    //获取用户对文件的权限
    mode_t mode ;
    */
    //判断文件是否打开
    int is_close ;
    //文件名称
    char name[LEN] ;
    //用户自定义缓冲区，发送文件的信息
    char buf[LEN] ;
} node_file ;

void set_socket(int& fd) ;
void process_request(int client_fd, node_file& file) ;
void recover_file(int client_fd, node_file& file) ;
void storage_file(int client_fd, node_file& file) ;
