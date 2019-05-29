#include"GetConfInfo.h"
#include"Client.h"
#include"GetLocalInfo.h"
using namespace std ;

int GetConfInfo(const char*confPath, char info[3][128]) {
    
    //文件不存在
    if(access(confPath, 0)) {
        return 0 ;
    }
    FILE* fp = fopen(confPath, "r") ;
    if(fp == NULL) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    char name[128] ;
    int i = 0 ;
    while(!feof(fp)) {
        fscanf(fp, "%s %s\n", name, info[i]) ;
        i++ ;
    }
    fclose(fp) ;
    return 1 ;
}

int GetConfPath(char info[3][128]) {
      
    char paths[PATH_MAX] ;
    const char*p = getcwd(paths, PATH_MAX) ;
    if(p == NULL) {
         printError(__FILE__, __LINE__) ;
         return 0 ; 
    }   
    std::string::size_type position ;
    //获取配置文件头部路径
    std::string s  = p ; 
    position = s.find("_deepin-file-control") ;

    if(position == s.npos) {
        printError(__FILE__, __LINE__) ;
        return 0;
    }   
    s[position] = '\0' ;
    strcpy(paths, s.c_str()) ;
    strcat(paths, "_deepin-file-control/conf/info") ;
    int ret = GetConfInfo(paths, info) ;
    if(ret == 0) {
        return 0 ;
    }
    return 1 ;
}

