#pragma once
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<stdio.h>
#include<errno.h>
int readn(int fd, void *buf, int n) ;
//写数据
int writen(int fd, void *buf, int n) ;

