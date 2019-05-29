#include"MsgQueue.h"

int IpcMsgCreate() {
    
    key_t key = ftok(MSG_FILE, MSG_ID) ;
    if(key < 0) {
        perror("ftok") ;
        return 0 ;
    }
    int msgid = msgget(key, IPC_CREAT|IPC_EXCL|0666) ;
    if(msgid < 0) {
        perror("msgget") ;
        return 0 ;
    }
    return msgid ;
}

int IpcMsgOpen() {

    key_t key = ftok(MSG_FILE, MSG_ID) ;
    int msgid = msgget(key, 0) ;
    if(msgid < 0) {
        perror("msgget") ;
        return 0 ;
    }
    return msgid ;
}

int IpcMsgSend(int msgid, Msg& data) {
    
    int ret = msgsnd(msgid, (void*)&data, sizeof(data)-sizeof(long), 0) ;
    if(ret < 0) {
        perror("msgsnd") ;
        return 0 ;
    }
    return 1 ;
}

int IpcMsgRecv(int msgid, Msg& data) {

    int ret = msgrcv(msgid, (void*)&data, sizeof(data)-sizeof(long), data.type,0) ;
    if(ret < 0) {
        perror("msgrcv") ;
        return 0 ;
    }
    return 1 ;
}
