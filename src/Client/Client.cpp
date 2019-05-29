#include"Client.h"
#include"MsgQueue.h"
#include"Judge.h"
#include"ReadWrite.h"
#include"GetLocalInfo.h"

//客户端默认接收消息为1的消息
//argv参数包含客户端要连接服务器的ip，端口
//要监控的目录

int ProcessHandle(char info[3][128]) {
    
    signal(SIGINT, SigHandle) ;
    int port = atoi(info[1]) ;
    int servFd = Connect(info[0], port) ;
    if(servFd == 0) {
        exit(1) ;
    }

    int msgId = IpcMsgCreate() ;
    if(msgId == 0) {
        return 0 ;
    }
    
    FreeInfo::msgId = msgId ;
    FreeInfo::servFd= servFd ;
    Msg msg ;
    while(1) {
        //设置为1，接收来自各个见客户端的请求
        msg.type = MSG_TYPE ;
        while(1) {
            if(IpcMsgRecv(msgId, msg) == 0) {
                return 0 ;
            }
            if(IsConnect(servFd, msgId, msg.buf.pid, info[0], port) == 0) {
                continue ;
            }
            std::thread t1(SendData, std::ref(msg), info[2], servFd, msgId) ;
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
    return fd ;
}


//向服务端发送请求
void SendData(Msg& msg, const char* monitorPath, int servFd, int msgId) {
    
    static map<string, int>counts ;
   //无论是打开文件还是关闭文件都判断是否为监控目录
    int ret = IsMonitorDir(msg.buf.pathName, monitorPath) ;
    //不是监控目录，向hook发送消息
    if(ret == 0) {
        msg.type = msg.buf.pid ;
        msg.buf.type = FINISH ;
        IpcMsgSend(msgId, msg);
        return ;
    }

    //判断文件是否存在
    if(IsExist(msg, msgId) == 0) {
        return ;
    }
    //文件在监控目录下，并且存在
    else {
        int type = msg.buf.type ;
        if(type == CLOSE) {
            //close请求判断是否为最后一次close请求,是最后一次close请求的
            //话,才恢复原来文件内容,否则不会恢复原来文件内容
            int ret =  RecoverRequest(servFd, msg, counts) ;
            if(ret < 0) {
                printError(__FILE__, __LINE__) ;
                return ;
            }
            //通知hook备份文件可以关闭了
            if(ret == 0) {
                msg.type = msg.buf.pid ;
                msg.buf.type = FINISH ;
                if(IpcMsgSend(msgId, msg) < 0) {
                    printError(__FILE__, __LINE__) ;
                    return ;
                }
            }
        }   
        if(type == OPEN) {
            int ret = SendFile(msg, servFd, counts) ;
            if(ret < 0) {
                printError(__FILE__, __LINE__) ;
                return ;
            }
            //返回值为0表示已经备份过该文件
            if(ret == 0) {
                msg.type = msg.buf.pid ;
                msg.buf.type = FINISH ;
                if(IpcMsgSend(msgId, msg) < 0) 
                    printError(__FILE__, __LINE__) ;
                return  ;
            }
        }
    }
}

//发送文件内容
int  SendFile(Msg msg, int servFd, map<string, int>&maps) {

    if(IsStoragedFile(msg, maps)) {
        return 0 ;
    }
    //判断是否为已经备份过的文件
    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = OPEN ;
    int ret = GetMac(data.mac) ;
    if(ret < 0 ) {
        return -1 ;
    }
    data.hookPid = msg.buf.pid ;
    strcpy(data.pathName, msg.buf.pathName) ;
    int fd = open(data.pathName, O_RDWR) ;
    if(fd < 0) {
        printError(__FILE__, __LINE__) ;
        return -1 ;
    }
    //客户端向服务器至少发送两次消息
    int counts = 0;
    long cur = 0 ; 
    while(1) {
        //读文件内容
        int ret = read(fd, data.buf, sizeof(data.buf)) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return -1 ;
        }

        //如果将文件读完了
        if(ret ==0&& counts) {
            data.type = OPEN ;
            memset(data.buf, 0, sizeof(data.buf)) ;
            //通知发送文件结束
            strcpy(data.buf, "EOF") ;
            //向服务器发送结束标志
            int res = writen(servFd, &data, sizeof(data)) ;
            if(res < 0) {
                printError(__FILE__, __LINE__);
                return -1 ;
            }
            close(fd) ;
            break ;
        }

        //设置偏移量
        data.left = cur ;
        cur += ret ;
        data.right = cur ;
        //向服务器发送文件内容
        int res =  writen(servFd, &data, sizeof(data)) ;
        if(res < 0) {
            Msg dd ;
            dd.type = msg.buf.pid ;
            dd.buf.type = FAIL ;
            if(IpcMsgSend(FreeInfo::msgId, dd) < 0) {
                printError(__FILE__, __LINE__) ;
                return -1 ;
            }
            printError(__FILE__, __LINE__) ;
            return  -1 ;
        }
        counts++ ;
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
    ret = writen(fd, data.buf, strlen((char*)data.buf)) ;
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
int RecoverRequest(int servFd, Msg msg, map<string, int>&recFile) {
    
    //mute::mute2.lock() ;
    //不是最后一次close请求或者是在map里寻找文件名失败
    if(!IsLastRecoverRequest(msg, recFile)) {
        if(msg.buf.pid == -1) {
            return -1 ;
        }
        return 0 ;
    }

    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = CLOSE ;
    int ret = GetMac(data.mac) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return -1 ;
    }
    data.hookPid = msg.buf.pid ;
    strcpy(data.pathName, msg.buf.pathName) ;
    if((ret = writen(servFd, &data, sizeof(data))) < 0) {
        printError(__FILE__, __LINE__) ;
        return -1 ;
    }
    return 1 ;
}

int RecoverFile(struct Data data, int& fd) {

    if(!strcmp((char*)data.buf,"EOF")) {
        close(fd) ;
        return 1 ;
    }

    lseek(fd, data.left, SEEK_SET) ;
    int ret = write(fd, data.buf, data.right-data.left) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        exit(1) ;
    }
    return 0 ;
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
        close(FreeInfo::servFd) ;
        msgctl(FreeInfo::msgId, IPC_RMID, 0) ;
        return ;
    }
}

