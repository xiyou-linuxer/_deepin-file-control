#include"Client.h"

int main(int argc, char** argv) {

    if(argc != 4) {
        printf("Usage:[./a.out] [ip] [port][dir_ab_name]\n");
        return 1 ;
    }

    if(argv[3][0] != '/') {
        printf("目录必须为绝对路径\n") ;
        return 1 ;
    }
    //开始处理
    ProcessHandle(argv) ;
}
