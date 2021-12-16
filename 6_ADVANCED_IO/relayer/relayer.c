#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "relayer.h"
#include <pthread.h>
#include <string.h>

#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"
#define BUFSIZE 1024

enum{
    STATE_R = 1,
    STATE_W ,
    STATE_Ex ,
    STATE_T 
};

struct rel_fsm_st {
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char* errstr;
    long long count;
};

struct rel_job_st {
    int job_state;
    int fd1;
    int fd2;
    struct rel_fsm_st fsm12, fsm21;
    int fd1_save, fd2_save;
};

int rel_addjob(int fd1 ,int fd2){

    // 初始化一次，启动线程轮训任务数组

    // 初始化 rel_job_st，设置非阻塞

    // 加锁数组job寻找位置并赋值
    

}

int rel_canceljob(int id){

}

int rel_waitjob(int id, struct rel_state_st* p){

}

int rel_statjob(int id, struct rel_state_st* p){

}
