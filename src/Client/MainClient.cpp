#include"Client.h"
#include"GetLocalInfo.h"
#include"GetConfInfo.h"

int main(int argc, char** argv) {
    
    char info[3][128] ;
    int ret  = GetConfPath(info) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 1 ;
    }
    
    if(info[2][0] != '/') {
        char msg[128] ;
        sprintf(msg, "配置路径必须为绝对路径   %s\n", __FILE__) ;
        printError(msg, __LINE__) ;
        return 1 ; 
    }

    char real_path[PATH_MAX] ;
    //过虑路径名
    if(!realpath(info[2], real_path)) {
        char msg[128] ;
        sprintf(msg, "监控路径不存在   %s\n", __FILE__) ;
        printError(msg, __LINE__) ;
        return 0 ;
    }
    
    int len = strlen(real_path) ;
    if(real_path[len-1] == '/') {
           real_path[len-1] = '\0' ;
    }
    //将真正的路径传给info
    strcpy(info[2], real_path) ;
    //开始处理
    ProcessHandle(info) ;
}


