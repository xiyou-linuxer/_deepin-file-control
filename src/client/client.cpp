#include <iostream>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<signal.h>
using namespace std ;
#define LEN 1024 
int file_fd ;

void handle(int signo) {
    
    if(signo == SIGINT) {
        close(file_fd) ;
    }
}

int main() {

    //打开文件，在此处劫持open函数，将文件信息发送给服务器
    //并对文见内容进行修改
    const char *file_name = "hello.c" ;
    file_fd = open(file_name, O_RDONLY) ;
    
    sighandler_t sig ;

    sig = signal(SIGINT, handle) ;

    if(file_fd<0) {
        perror("open") ;
        exit(1) ;
    }
    //在此处读到修改后的内容
    char buf[LEN] ;
    int res ;

    cout << "hook 后的文件内容：" << endl ;
    lseek(file_fd, 0, 0) ;
    while((res = read(file_fd, buf, sizeof(buf)))) {
        
        if(res < 0) {
            perror("read") ;
            exit(1) ;
        }
        cout << buf << endl ;
        if(res < LEN) break ;
    }

    close(file_fd) ;
    return 1;
}


