#pragma once
#include<map>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<iostream>
#include"MsgQueue.h"
using namespace std ;
int  IsStoragedFile(Msg msg, map<string, int>&maps) ;
//判断问价是否文系统已经存在的文件
//要是系统存在的文件，返回，不是的话，
//根据hook所给的权限进行创建文件，创建成功返回1，继续下面判断文件是否
//是否时间空目下的文件；创建不成功就将文件的错误发回给hook
int IsExist(Msg msg, int msgId) ;
//验证是否为监控的目录
int IsMonitorDir(const char*hookPath, const char* monitorPath) ;
//判断是否为最后一次发送恢复文件请求
int IsLastRecoverRequest(Msg& msg, map<string, int>&recFile) ;
