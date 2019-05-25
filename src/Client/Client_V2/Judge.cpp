#include"Judge.h"
#include"Client.h"
int IsExist(Msg msg, int msgId) {

    int fd = -1 ;
    struct stat st ;
    int flag = 0 ;
    //判断文件存不存在
    int ret = stat(msg.buf.pathName, &st) ;
    if(ret < 0) {
        if(errno == ENOANO) {
            flag = 1 ;
        }
    }
    //如果文件不存在
    if(flag == 1) {
        if(msg.buf.mode == 0) {
            fd = open(msg.buf.pathName, msg.buf.flag) ;
        }
        else {
            fd = open(msg.buf.pathName, msg.buf.flag, msg.buf.mode) ;
        }
        //创建文件失败：可能是路径错误或者是权限错误,直接给hook
        //程序发送备份文件失败，让hook中的open返回-1
        if(fd < 0) {
            Msg data ;
            memset(&data, 0, sizeof(data)) ;
            data.type = FAIL ;
            data.type = data.buf.pid ;
            //通过消息队列将消息发送给hook，hook收到就返回 -1 
            if(IpcMsgSend(msgId, data) == 0) {
                return 0 ;
            }
        }
    }
    close(fd) ;
    return 1 ;
}

//验证是否为监控的目录
int IsMonitorDir(const char* hookPath, const char* monitorPath) {
    
    int i = 1 ;
    while(1) {
        //判断绝对路径第一个目录名一致的话，就辨明在监控目录下
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

//判断文件是否已经备份过了
//备份过的话,就不用给服务器发送消息了
//否则的话,往map里面增加元素,并给服务器发送消息
int IsStoragedFile(Msg msg, map<string, int>&stFiles) {

    map<string, int>::iterator iter = stFiles.find(msg.buf.pathName);
    //找到的话,就将文件的值加1
    if(iter != stFiles.end()) {
         iter->second++ ;   
         return 1 ;
    }
    //没找到的话,就创建一个pair
    else {
        pair<string, int> tmp  ;
        tmp.first = msg.buf.pathName ;
        tmp.second = 1 ;
        stFiles.insert(tmp);
        return 0 ;
    }
    map<string, int>::iterator iters;
    for(iters = stFiles.begin(); iters != stFiles.end(); iters++) {
        printf("key=%s    value=%d\n", iters->first.c_str(), iters->second) ;
    }
}

//判断是否是最后一次close请求
int IsLastRecoverRequest(Msg& msg, map<string, int>&recFile) {
        
    map<string, int>::iterator iter = recFile.find(msg.buf.pathName);
    if(iter != recFile.end()) {
         iter->second -- ;          
    }
    //如果没找到该路径名称,将pid置为-1
    else {
        msg.buf.pid = -1 ;
        return 0 ;
    }
    //如果是最后一次close,经客户端的文件备份记录删掉
    if(iter->second == 0) {
        recFile.erase(iter) ;
        return 1 ;
    }
    return 0 ;
}