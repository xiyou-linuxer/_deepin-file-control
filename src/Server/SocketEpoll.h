#ifndef __SOCKET_EPOLL_H_
#define __SOCKET_EPOLL_H_
#include "model.h"
//线程传递
struct fds {
     int epollfd;
     int sockfd;
public:
    fds(int epollfd, int sockfd){} 
};
//socket函数
class Socket {
	public:
		Socket() {}
		/*Socket(int _epollfd,int _port,const char *_ip) {
			epollfd = _epollfd;
			port = _port;
			if(_ip != NULL) {
				ip = new char [strlen(_ip)];
				strcpy(ip,_ip);
			}
			else 
				ip = NULL;
		}
		Socket(int _epollfd,int _listenfd,bool _enable) {
			epollfd = _epollfd;
			lfd = _listenfd;
			enable = _enable;
		}*/
		~Socket() {}
    public:
        int create_listenfd(int epollfd,int port, char *ip);
        int create_connfd(int epollfd,int listenfd,bool enable);
        void add_fd(int epollfd,int fd,bool enable);
        void set_noblock(int fd);
		void reset_shot(int epollfd,int fd);
    private:
        int lfd;
        int cfd;
		int port;
		char *ip;
        int epollfd;
        bool enable;
};
void action(struct epoll_event * events,int nums,int efd,int lfd,Socket s);

#endif
