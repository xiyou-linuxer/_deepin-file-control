#include"open_close.h"

int change_file() {
    
    int ret = 0 ;
    
    //表示文件未关闭
    file.is_close = 0 ;

    cout << "修改文件内容......" << endl ; 
    
    //读文件,并发给服务器
    while((ret = read(file.fd, file.buf, sizeof(file.buf)))) {
        if(ret < 0) {
            return -1 ;
        }

        cout << file.buf << endl ; 
        //将整个结构体发给服务器
        int res = send(file.serv_fd, &file, sizeof(file), 0) ;
        
        if(res < 0) {
            return -1 ;
        }
            
        if(ret < LEN) break ;
    }

    //修改文件操作权限
    if(ret < 0) {
        perror("fcntl") ;
        return -1 ;
    }
    
    //修改文件内容
    //获取文件的长度
    ftruncate(file.fd, 0) ;
    const char* buf="It's a secret!" ;
    int res = write(file.fd, buf, strlen(buf)) ;
    if(res < 0) {
        perror("write") ;
        return -1 ;
    }
    cout <<"加入成功" <<endl ;
    return 1 ;
}

//用户连接服务器
void user_connect() {
    
    int fd = socket(AF_INET, SOCK_STREAM, 0) ;
    struct sockaddr_in cliaddr ;
    cliaddr.sin_family = AF_INET ;
    cliaddr.sin_port = htons(PORT) ;    

    int res = inet_aton(IP, &cliaddr.sin_addr) ;
    
    if(res < 0) {
        perror("inet_aton") ;
        exit(1) ;
    }

    if(connect(fd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) {
        perror("connect error!") ;
        exit(1) ;
    }

    //获取服务端套接字
    file.serv_fd = fd ;
}

int open(const char* pathname, int flag, ...) {
    
    //记录文件信息，将该结构传给服务器
    bzero(&file, sizeof(file)) ;
    file.flag = flag ;

    user_connect() ;

    //获取动态共享库的
    //句柄及相关函数在共享库中的地址
    func_open real_open ;
    void* handle ;
    char* error ;
    handle = dlopen("libc.so.6", RTLD_LAZY) ;
        
    if((error = dlerror()) != NULL) {
        puts(error) ;
        exit(1) ;
    }

    real_open = (func_open)dlsym(handle, "open") ;
    
    //在hook住open后在函数中读一遍文件
    int ret = real_open(pathname, flag|O_RDWR) ;
    if(ret < 0) {
        perror("open") ;
        exit(1) ;
    }

    int res ;
    strcpy(file.name, pathname) ;
    
    file.fd = ret ;
    //将文件内容发送给服务器 
    //如果修改文件失败
    //直接给客户返回open调用失败
    res = change_file() ;
    if(res < 0) {
        close(file.fd) ;
        return res ;
    }

    //将文件的操作权限改回原用户所请求的权限
    res = fcntl(file.fd, F_SETFD, flag) ;

    if(res < 0) {
        close(file.fd) ;
        return res ;
    }

    return ret ;
}

//关闭文件
int close(int fd) {
    
    file.fd = fd ;
    func_close real_close ;
    void* handle = dlopen("libc.so.6", RTLD_LAZY) ;
    char* error ;
    if((error = dlerror()) != NULL) {
        perror(error) ;
        exit(1) ;
    }
    real_close = (func_close)dlsym(handle, "close") ;

    int ret ;
    //给服务器发送消息让其恢复文件内容
    recover_file_info() ;

    //关掉服务端套接字
    real_close(file.serv_fd) ;
    
    ret = real_close(file.fd) ;
    return ret ;
}

//服务器端要接收请求将文件内容传给客户端
int recover_file_info() {
    
    //表示文件关闭
    file.is_close = 1 ;

    int res = send(file.serv_fd, &file, sizeof(file), 0) ;
    if(res < 0) {
        //close(file.serv_fd) ;     
        return -1 ;
    }

    //该文件权限为追
    //当接收到服务器端的消息出错时如何处理？？？？？
    ftruncate(file.fd, 0) ;
    cout << "原文件真实内容：" << endl ;
    while(1) {

        res = recv(file.serv_fd, &file, sizeof(file), 0) ;
        if(res < 0) {
            perror("recv") ;
            return -1 ;
        }
        cout << file.buf << endl ;
        write(file.fd, file.buf, strlen(file.buf)) ;
        if(res == 0) break ;
        if(res < LEN) {
            break ;
        }
    }
    return 1 ;
}
