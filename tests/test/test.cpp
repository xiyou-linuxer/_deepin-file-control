#include <iostream>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>

#define LEN 1024
int main(int argc,char *argv[])
{
    int fd = open("test", O_RDWR) ;

    if(fd < 0) {
        exit(1) ;
    }
    printf("修改文件内容完成，继续恢复文件内容...\n") ;
    getchar();
    close(fd) ;
    return 0;
}

