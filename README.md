## 项目概况

###  背景
- 项目来源：[2019 深度软件开发大赛](https://www.deepin.org/devcon-2019/topic)
- 项目名称：文件管控客户端

### 运行环境
- deepin Linux x86_64 系统，理论上也兼容其他 x86_64 Linux 系统

### 条件与限制
- 区分服务端与客户端，一般运行在不同的机器上
- 正式的运行环境是N(N>=2)台计算机节点，通过有线或者无线互联，且运行服务端机器有客户端机器能直连的IP地址

### 业务功能需求
- 监视和记录指定目录下文件的打开和关闭动作（文件系统事件），并上报服务端（文件路径、文件句柄、操作方式等）
- 配置了指定的监视服务器情况下，可接受服务端下发的文件操作指令，包括不限于读取、删除、重写等
- 当监视服务器关闭或者网络不通时，客户端拒绝任何操作，不允许任何人打开文件（拒绝监视目录的一切文件操作）
- 监视服务器不存在或者未配置时，记录相关的操作到日志文件，监视服务器可以查询历史操作日志

### 相关知识点
 - hook技术原理以及cpu运行级别。
 - 系统调用过程（open的具体调用过程）。
 - 网络编程。
 - Linux文件系统
 - 日志管理。


##  1.设计决策
##  2.逻辑架构设计
##  3.接口设计

### 1.设计决策
 - 使用Linux C系统编程，网络编程技术进行分析与设计，并使用uml描述系统的设计模型
 - 使用C/C++语言进行开发
 - 逻辑架构采用事件驱动架构
 - 物理架构采用客户端，服务器模式的C/S架构
 - 数据采用集中式存储，使用文件存储

### 2.逻辑架构设计

#### 2.1 总体逻辑架构
![图片](https://github.com/xiyou-linuxer/_deepin-file-control/blob/master/images/zonti.png)
#### 2.2 时序图
![图片](https://github.com/xiyou-linuxer/_deepin-file-control/blob/master/images/shi_xu.png)
#### 2.3 open请求流程图
![图片](https://github.com/xiyou-linuxer/_deepin-file-control/blob/master/images/open.jpg)
#### 2.4 close请求流程图
![图片](https://github.com/xiyou-linuxer/_deepin-file-control/blob/master/images/close.jpg)
#### 2.5 关键问题
##### 2.5.1.多个hook进程同时访问客户端维护的一条消息队列
     策略：使用特殊元素区分
     技术：不同进程添加接收消息字段时将消息类型设置为进程id，通过不同的id进行区别
##### 2.5.2 服务端备份数据
     策略：使用别名存储
     技术：通过mac地址与文件路径生成笔名，存储在文件中，使用map提取

### 3.接口设计
#### 3.1 所有宏定义
```c
    //打开文件请求
    #define OPEN 1
    //关闭文件请求
    #define CLOSE 2
    //发送文件成功
    #define FINISH 3
    //判断是否还连接
    #define ALIVE 4
    //不是监控目录
    #define INVAILD 5
    //存MAC地址数组的长度
    #define MAC_LEN 128
    //消息队列的路径
    #define MSG_FILE "."
    //数据缓冲区的长度
    #define BUF_SIZE (4*1024)
```
#### 3.2 传输文本格式
```c    
struct Packet {
       //消息请求类型
       int type;
       //客户端pid
       pid_t pid ;
       //文件路径
       char pathName[PATH_MAX] ;
    };
```
#### 3.3 消息队列传输消息类型
```c    
struct Msg {
        //消息类型
        long type ;
        //消息数据
        Packet buf ;
    }; 
```
#### 3.4 客户端与服务器之间的消息数据

```c    
struct Data {
        //客户端请求类型
        int type ;
        //客户端主机的mac地址
        char mac[MAC_LEN] ;
        //文件路径名
        char pathname[PATH_MAX] ;
        //数据缓冲区
        char buf[BUF_SIZE] ;
    };
```

#### 3.5 客户端主要函数设计

##### 3.5.1 client.h 处理客户端业务的头文件

函数名|参数|功能描述|返回值
--|:--:|--:|--:
ProcessHandle|char** argv|客户端处理业务的入口|int
Connect |const char* ip, const int port|连接服务器|int
IsMonitorDir|const char * HookPath, const char * monitorPath|验证是否为监控的目录  |int
 SendData|Msg msg, const char * monitorPath, int fd, int msgid|向服务端发送请求  |void 
RecvData|Msg msg, const char * monitorPath, int fd, int msgid|接收服务端请求  |void
Alive|int fd|判断与服务器是否断开连接  |int


##### 3.5.3 msgQueue.h 处理消息队列的头文件

函数名|参数|功能描述|返回值
--|:--:|--:|--:
IpcMsgCreate|void|创建消息队列 |int
IpcMsgOpen|void|打开已有的消息队列  |int
IpcMsgSend|int msgid, Msg data|向消息队列中添加消息  |int
IpcMsgRecv|int msgid, Msg data|从消息队列中接收消息   |int
IpcFree|int msgid|删除消息队列  |int

#### 3.６ 服务器端主要函数设计

函数名|参数|功能描述|返回值
--|:--:|--:|--:
SrvListenfdCreate|int epollfd,int port,const char*IP|创建监听套接字并加入epoll结构 |int
SrvConfdCreate|int epollfd,int listenfd,bool enable|创建读写套接字并加入epoll结构|int
SrvAddFdToEpoll|int epollfd,int fd|添加描述符至epoll结构|void
SrvFileOptions|int connfd,struct packet &buf|数据处理操作|void
