#include"server.h"


int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0) ;
    
    if(fd < 0) {
        perror("socket") ;
    }

    set_socket(fd) ;
    
    int client_fd = -1 ;

    while(1) {
        
        client_fd = accept(fd, (struct sockaddr*)NULL, NULL) ;
        if(client_fd < 0) {
            perror("accept") ;
            exit(1) ;
        }
        cout << "客户端连接成功" << endl ;
        node_file file ;
        
        //开线程处理客户端请求
        thread task(process_request, client_fd, ref(file));
        task.detach() ;
    }

    return 0;
}

void process_request(int client_fd, node_file& file) {
    
    while(1){
        int ret = recv(client_fd, &file, sizeof(file), 0) ;
        if(ret < 0) {
            perror("recv") ;
            close(client_fd) ;
            pthread_exit(0) ;
        }

        if(ret == 0) {
            recover_file(client_fd, file) ;
            close(client_fd) ;
            break ;
        }

        file.cb_num = ret ;
        //根据文件状态处理
        switch(file.is_close) {
        
            //客户端发来关闭文件的消息，服务器端恢复客户端原文件
            case 1 :
                recover_file(client_fd, file) ;
                break ;
            //客户端发来打开文件的消息，服务器端备份文件
            case 0 :
                storage_file(client_fd, file) ;
                break ;
            default :
                break ;
        }
        cout << "OK" << endl ;
    }
}

//恢复文件
void recover_file(int client_fd, node_file& file) {
    
    ifstream input(file.name) ;
    if(!input) {
        cout << "文件没法打开..." << endl ;
        exit(1) ;
    }

    while(!input.eof()) {

        memset(file.buf, 0, sizeof(file.buf)) ;
        input >> file.buf ;
        int ret = send(client_fd, &file, sizeof(file), 0) ;
        if(ret < 0) {
            cout << "恢复文件失败..." << endl ;
            exit(1) ;
        }

        if(ret < LEN) {
            break ;
        }
    }

    input.close() ;
    //恢复完就将原文件删掉
    remove(file.name) ;
}

//备份文件
void storage_file(int client_fd, node_file& file) {
    
    //创建文件
    int res = 0 ;
    cout << file.name <<"\n"<<endl ;
    cout << file.buf <<endl ;
    ofstream out(file.name) ;
    out << file.buf ;

    if(file.cb_num+1 == LEN) {
        
        cout << file.cb_num << endl ;
        
        while((res = recv(client_fd, &file, sizeof(file), 0))) {
            
            if(res < 0) {
                perror("客户端异常断开") ;
                exit(1) ;
            }

            //将文件内容拷贝到服务器端同名文件中
            cout<< file.buf <<endl ;
            out << file.buf;
        
            //recv返回值小于buf的长度，表明对端将文件读到末尾
            //接收完毕
            if(res < LEN) {
                break ;
            }
        }
    }
    out.close() ;
    cout << "文件备份" << endl ;
}

void set_socket(int& fd) {

    struct sockaddr_in servaddr ;
    servaddr.sin_family = AF_INET ;
    servaddr.sin_port = htons(2345) ;
    servaddr.sin_addr.s_addr =  inet_addr(IP) ;
    int opt = 1 ;

    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int)) < 0) {
        perror("setsockopt!") ;
        exit(1) ;
    }

    if((bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
        perror("bind") ;  
        exit(1) ;
    }
    
    if(listen(fd, 10) < 0) {
        perror("listen") ;
    }
}
