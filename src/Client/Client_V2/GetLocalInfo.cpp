#include"GetLocalInfo.h"

void printError(const char* fileName, const int line) {

    time_t t ;
    struct tm* lt ;
    time(&t) ;
    lt = localtime(&t) ;
    char buf[1024] ;
    sprintf(buf, "%d/%d/%d %d:%d:%d  ", lt->tm_year+1900, 
            lt->tm_mon,lt->tm_mday, lt->tm_hour,
            lt->tm_min, lt->tm_sec) ;
    int fd = open("syslog/log", O_WRONLY|O_APPEND) ;
    if(fd < 0) {
        exit(1) ;
    }
    char info[LEN] ;
    sprintf(info, "出错文件名：%s，出错行号：%d\n", fileName, line) ;
    write(fd, buf, strlen(buf)) ;
    write(fd, info, strlen(info)) ;
    close(fd) ;
}

int GetMac(char* macAddr) {
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[2048];

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) {
        printf("socket error\n");
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) {
        printError(__FILE__, __LINE__) ;
        return 0;
    }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
    char szMac[64];
    int count = 0 ;
    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    count ++ ;
                    unsigned char * ptr ;
                    ptr = (unsigned char  *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
                    snprintf(szMac,64,"%02X:%02X:%02X:%02X:%02X:%02X",
                             *ptr,*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5));
                    sprintf(macAddr ,"%s",szMac);
                }
            }
        }
        else{
            printf("get mac info error\n");
            return 0;
        }
    }
    return 1 ;
}

