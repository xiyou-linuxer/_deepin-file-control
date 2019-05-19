#include"Client.h"
#include"MsgQueue.h"
#include"GetLocalInfo.h"
#include"ReadWrite.h"

using namespace Keep_Alive ;
using namespace MsgId ;
using namespace ServSock ;
//argv参数包含客户端要连接服务器的ip，端口
//要监控的目录

int ProcessHandle(char** argv) {
    
    signal(SIGINT, SigHandle) ;
    int port = atoi(argv[2]) ;
    int servFd = Connect(argv[1], port) ;
    if(servFd == 0) {
        exit(1) ;
    }
    
    int msgId = IpcMsgCreate() ;
    if(msgId == 0) {
        return 0 ;
    }
    
    MsgId::msgId = msgId ;
    Msg msg ;
    while(1) {
        //设置为1，接收来自各个见客户端的请求
        msg.type = MSG_TYPE ;
        while(1) {
            if(IpcMsgRecv(msgId, msg) == 0) {
                return 0 ;
            }
            std::thread t1(SendData, std::ref(msg), argv[3], servFd, msgId) ;
            std::thread t2(RecvData, servFd, msgId) ;
            t1.detach() ;
            t2.detach() ;
        }
    }
}

//连接服务器
int Connect(const char* ip, const int port) {
    
    int fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if(fd < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET ;
    addr.sin_port = htons(port) ;
    int ret = inet_aton(ip, &addr.sin_addr) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    
    ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr)) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    ServSock::servsock = fd ;
    return fd ;
}


//验证是否为监控的目录
int IsMonitorDir(const char* hookPath, const char* monitorPath) {
    
    int i = 1 ;
    while(1) {
        
        if(hookPath[i] == '/' && monitorPath[i] == '/') {
            break ;
        }
        if(hookPath[i] != monitorPath[i]) {
            return 0 ;
        }
        i++ ;
    }
    return 1 ;
}

//向服务端发送请求
void SendData(Msg& msg, const char* monitorPath, int servFd, int msgId) {

    if(IsExist(msg, msgId) == 0) {
        return ;
    }
    //无论是打开文件还是关闭文件都判断是否为监控目录
    int ret = IsMonitorDir(msg.buf.pathName, monitorPath) ;
    //不是监控目录，向hook发送消息
    if(ret == 0) {
        msg.type = msg.buf.pid ;
        msg.buf.type = FINISH ;
        IpcMsgSend(msgId, msg);
        return ;
    }
    else {
        int type = msg.buf.type ;
        if(type == CLOSE) {
            int ret =  RecoverRequest(servFd, msg) ;
            if(ret < 0) {
                printError(__FILE__, __LINE__) ;
                return ;
            }
        }
        if(type == OPEN) {
            int ret = SendFile(msg, servFd) ;
            if(ret < 0) {
                printError(__FILE__, __LINE__) ;
                return ;
            }
        }
    }
}

//发送文件内容
int  SendFile(Msg msg, int servFd) {

    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = OPEN ;
    int ret = GetMac(data.mac) ;
    if(ret < 0 ) {
        return 0 ;
    }
    data.hookPid = msg.buf.pid ;
    strcpy(data.pathName, msg.buf.pathName) ;
    int fd = open(data.pathName, O_RDWR) ;
    if(fd < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    while(1) {
        //读文件内容
        ret = read(fd, data.buf, sizeof(data.buf)) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return 0 ;
        }
        //如果将文件读完了
        if(ret == 0) {
            data.type = OPEN ;
            memset(data.buf, 0, sizeof(data.buf)) ;
            //向服务器发送文件内容
            int res = writen(servFd, &data, sizeof(data)) ;
            if(res < 0) {
                printError(__FILE__, __LINE__);
                return 0 ;
            }
            close(fd) ;
            break ;
        }
        //向服务器发送文件内容
        int res =  writen(servFd, &data, sizeof(data)) ;
        if(res < 0) {
            printError(__FILE__, __LINE__) ;
            return  0 ;
        }
    }
    return 1 ;
}   

//接收服务端请求
int RecvData(int servFd, int msgId) {

    struct Data data ;
    int fd = -1 ;
    int ret ,res ;
    int count = 0 ;
    while(1) {
        memset(&data, 0, sizeof(data)) ;
        ret = readn(servFd, &data, sizeof(data)) ;
        //要是第一次open请求或者close请求，打开文件
        //其他情况下只写文件
        if(count == 0){  
            fd = GetFileFd(data) ;
            if(fd == 0) {
                printError(__FILE__, __LINE__) ;
                return 0 ;
            }
            count ++ ;
        }
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            break ;
        }
        switch(data.type) {
        case OPEN :
            res = SendHookMsg(data, msgId, fd) ;
            if(res == 0) {
                printError(__FILE__, __LINE__) ;
                exit(1) ;
            }
            close(fd) ;
            return 0;
        case CLOSE :
            res = RecoverFile(data, fd) ;
            if(res == 1) {
                Msg msg ;
                msg.type = data.hookPid ;
                msg.buf.type =FINISH ;
                IpcMsgSend(msgId, msg) ;
                close(fd) ;
                return 0;
            }
            break ;
        }
    }
    return 0;
}

//向hook程序发送消息
int SendHookMsg(struct Data data, int msgId, int& fd) {

    int ret = -1 ;
    ret = writen(fd, data.buf, strlen(data.buf)) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    Msg msg ;
    //将客户端返回的结果返回给hook
    msg.buf.type = data.type ;
    msg.type = data.hookPid ;
    //给hook发消息
    if(IpcMsgSend(msgId, msg) < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    return 1 ;
}

//发送恢复文件请求
int RecoverRequest(int servFd, Msg msg) {
    
    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = CLOSE ;
    int ret = GetMac(data.mac) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    data.hookPid = msg.buf.pid ;
    strcpy(data.pathName, msg.buf.pathName) ;
    if((ret = writen(servFd, &data, sizeof(data))) < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    return 1 ;
}

int RecoverFile(struct Data data, int& fd) {
    
    if(!strlen(data.buf) || !strcmp(data.buf,"EOF")) {
        close(fd) ;
        return 1 ;
    }

    int ret = writen(fd, data.buf, strlen(data.buf)) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        exit(1) ;
    }
    return 0 ;
}

int IsExist(Msg msg, int msgId) {

    int fd = -1 ;
    //根据路径判断该文件是否存在,不存在的话就创建一个文件
    if(access(msg.buf.pathName, F_OK) < 0 && msg.buf.type == OPEN) {
        //文件不存在创建一个新文件根据发过来的权限
        fd = open(msg.buf.pathName, msg.buf.flag, msg.buf.mode) ;
        if(fd < 0) {
            Msg data ;
            //文件不存在的话，就为hook创建一个文件，创建失
            //败通过消息发消息通知hook进程退出
            memset(&data, 0, sizeof(data)) ;
            data.type = INVAILD ;
            data.type = data.buf.pid ;
            if(IpcMsgSend(msgId, data) == 0) {
                return 0;
            }
        }
    }
    close(fd) ;
    return 1;
}

//获取文件打开的fd
int GetFileFd(Data data) {
    int fd = 0;
    if(data.type == CLOSE) {
        fd = open(data.pathName, O_WRONLY) ;
        ftruncate(fd, 0) ;
        lseek(fd, 0, 0) ;
        if(fd < 0) {
            printError(__FILE__, __LINE__) ;
            return 0 ;
        }
        return fd ;
    }

    else if(data.type == OPEN) {
        fd = open(data.pathName, O_WRONLY) ;
        ftruncate(fd, 0) ;
        lseek(fd, 0, 0) ;
        if(fd < 0) {
            printError(__FILE__, __LINE__) ;
            return 0 ;
        }
        return fd ;
    }
    else {
        return 0 ;
    }
}

//接收到异常信号
void SigHandle(int signo) {

    if(signo == SIGINT) {
        close(ServSock::servsock) ;
        msgctl(MsgId::msgId, IPC_RMID, 0) ;
        return ;
    }
}

//记录日志
void printError(const char* fileName, const int line) {
    int fd = open("syslog/log", O_WRONLY|O_APPEND) ;
    if(fd < 0) {
        exit(1) ;
    }
    char info[LEN] ;
    sprintf(info, "出错文件名：%s，出错行号：%d", fileName, line) ;
    write(fd, info, strlen(info)) ;
    close(fd) ;
}


