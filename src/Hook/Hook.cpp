#include <iostream>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

#define FILEPATH "/tmp"
#define PROJID 0x123

#define OPEN 1  //open请求
#define CLOSE 2 //close请求
#define FINISH 3//文件修改完成应答
#define ALIVE 4 //客户端与服务器连接断开应答
#define INVALID 5//文件非监控目录应答
#define ACCESS 6//返回文件是否存在应答

//消息内容
struct packet {
    //消息请求类型
    int type;
    //客户端id
    pid_t pid;
    //对文件的操作
    int flag;
    //操作权限
    mode_t mode;
    //文件路径
    char pathname[PATH_MAX];

};
//消息队列格式的包
struct msg_queue {
    //消息类型
    long type;
    //消息数据
    struct packet buf;
};

//得到队列标识符
int getkey()
{
    int msgid;
    key_t key;

    key = ftok(FILEPATH, PROJID);
    if (key == -1) {
        perror("ftok()");
        exit(1);
    }

    msgid = msgget(key, 0);
    if (msgid == -1) {
        perror("msgget()");
        exit(1);
    }

    return msgid;
}

//根据文件描述符获取文件绝对路径
void get_file_name (const int fd,char *path) 
{
    char buf[1024] = {'\0'};
    snprintf(buf, sizeof (buf), "/proc/self/fd/%d", fd);
    readlink(buf, path, 100);
}

typedef int(*Open)(const char *pathname,int flags,...);

int open(const char *s1,int flags,...)
{       
    mode_t tmp = 0;
    if(flags & O_CREAT) {
        //获取第三个参数
        va_list va;
        va_start(va,flags);
        tmp = va_arg(va,int); 
        va_end(va);
    }
    printf("open(%s,%d,%d)\n",s1,flags,tmp);


    struct msg_queue msg;
    //根据文件相对路径获取绝对路径

    realpath(s1,msg.buf.pathname);
    msg.buf.flag = flags;
    msg.buf.mode = tmp;

    int msgid = getkey();

    //发送OPEN请求到消息队列
    
    msg.type = 1;
    msg.buf.pid = getpid();
    msg.buf.type = OPEN;
    
    printf("发送OPEN请求:文件绝对路径:%s 消息队列消息类型%ld 进程ID%d\n",msg.buf.pathname,msg.type,msg.buf.pid);


    if (msgsnd(msgid, (void *)&msg, sizeof(msg) - sizeof(long), 0) < 0) {
        perror("msgsnd()");
        exit(1);
    }



    //已将open请求发送给客户端,等待客户端返回通知
    struct msg_queue Recv_msg;

    while(1) {
        int i = 0;
        if((i = msgrcv(msgid,&Recv_msg,sizeof(Recv_msg) - sizeof(long),getpid(),0)) > 0) {
            printf("\nOPEN:客户端返回消息类型:");
            switch(Recv_msg.buf.type) {
                case ALIVE:
                    printf("alive\n");
                    //服务器与客户端连接已断开，拒绝任何程序打开监控目录下的所有文件
                    return -1;
                case INVALID:
                    printf("invalid\n");
                    //文件非监控目录下,直接返回其描述符
                    break;
                case FINISH:
                    printf("finish\n");
                    //服务器已备份完文件内容，可返回文件描述符
                    break; 
                case ACCESS:
                    printf("access\n");
                    return -1;
            }
        }
        if(i)
            break;
    }
    
    static void *handle = NULL;
    static Open old_open = NULL;

    if(!handle) {
        handle = dlopen("libc.so.6",RTLD_LAZY);
        old_open = (Open)dlsym(handle,"open");
    }

    if(tmp) {
        printf("open2\n");
        return old_open(s1,flags,tmp);
    }
    else {
        printf("open\n");
        return old_open(s1,flags);   
    }
}

typedef int(*Close)(int fd);

int close(int fd)
{
    int msgid = getkey();
    struct msg_queue msg;

    //发送CLOSE请求到客户端

    msg.type = 1;;
    msg.buf.pid = getpid();
    msg.buf.type = CLOSE;

    memset(msg.buf.pathname,0,100);

    get_file_name(fd,msg.buf.pathname);

    
    printf("发送CLOSE请求:文件绝对路径:%s 消息队列消息类型%ld 进程ID%d\n",msg.buf.pathname,msg.type,msg.buf.pid);
    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd()");
        exit(1);
    }

     //已将close请求发送给客户端,等待客户端返回通知
    struct msg_queue Recv_msg;
    
    if(msgrcv(msgid,&Recv_msg,sizeof(Recv_msg) - sizeof(long),getpid(),0) != -1) {
        printf("\nCLOSE:客户端返回消息类型:");
        switch(Recv_msg.buf.type) {
            case ALIVE:
                printf("alive\n");
                //服务器与客户端连接已断开，拒绝任何程序关闭监控目录下的所有文件
                return -1;
            case FINISH:
                printf("finish\n");
                //服务器已恢复完文件内容，可正常返回
                break;
            case ACCESS:
                printf("access\n");
                //文件非监控目录，正常返回
                break;
        }
    }
    
    static void *handle = NULL;
    static Close old_close = NULL;

    if(!handle) {
        handle = dlopen("libc.so.6",RTLD_LAZY);
        old_close = (Close)dlsym(handle,"close");
    }
    return old_close(fd);
}
