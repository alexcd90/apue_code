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

static pthread_mutex_t mutex_job = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t thread_once = PTHREAD_ONCE_INIT;
static struct rel_job_st* job_max[REL_JOBMAX];

enum
{
    STATE_R = 1,
    STATE_W,
    STATE_Ex,
    STATE_T
};

struct rel_fsm_st
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *errstr;
    long long count;
};

struct rel_job_st
{
    int job_state;
    int fd1;
    int fd2;
    struct rel_fsm_st fsm12, fsm21;
    int fd1_save, fd2_save;
};

static void fsm_driver(struct rel_fsm_st *fsm)
{
    int ret;
    // switch切换状态运行
    switch (fsm->state)
    {
    case STATE_R:
        //
        fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
        // len == 0
        if (fsm->len == 0)
        {
            fsm->state = STATE_T;
        }
        // len < 0 异常情况
        else if (fsm->len < 0)
        {
            if (errno == EAGAIN)
            {
                fsm->state = STATE_R;
            }
            else
            {
                fsm->errstr = "read() err";
                fsm->state = STATE_Ex;
            }
        }
        else
        {
            // len > 0 正常读取有数据
            fsm->state = STATE_W;
            fsm->pos = 0;
        }
        break;
    case STATE_W:
        //
        ret = write(fsm->dfd, fsm->buf + fsm->pos, fsm->len);
        // ret < 0 write异常
        if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                fsm->state = STATE_W;
            }
            else
            {
                fsm->errstr = "write() err";
                fsm->state = STATE_Ex;
            }
        }
        else
        {
            // len >= 0
            fsm->len -= ret;
            fsm->pos += ret;
            // len == 0 W >>  R
            if (fsm->len == 0)
            {
                fsm->state = STATE_R;
            }
            else
            {
                // len > 0  W >>> W
                fsm->state = STATE_W;
            }
        }
        break;
    case STATE_Ex:
        // Ex => T
        perror(fsm->errstr);
        fsm->state = STATE_T;
        break;
    case STATE_T:
        /****do sth ***/
        break;
    default:
        /****do sth ***/
        abort();
        break;
    }
}

static void *thr_func(void *p)
{
    while (1)
    {
        pthread_mutex_lock(&mutex_job);
        for (int i = 0; i < REL_JOBMAX; i++)
        {
            if (job_max[i] != NULL)
            {
                if (job_max[i]->job_state == STATE_RUNNING)
                {
                    fsm_driver(&job_max[i]->fsm12);
                    fsm_driver(&job_max[i]->fsm21);
                    if (job_max[i]->fsm12.state == STATE_T || job_max[i]->fsm21.state == STATE_T)
                    {
                        job_max[i]->job_state = STATE_OVER;
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex_job);
    }
}

static void module_load(void)
{
    // 创建线程执行循环处理
    pthread_t tid;
    int err;
    err = pthread_create(&tid, NULL, thr_func, NULL);
    if (err)
    {
        fprintf(stderr, "pthread_create():%s\n", strerror(err));
        exit(1);
    }
    
}

static int get_free_pos_unlock()
{
    for (int i = 0; i < REL_JOBMAX; i++)
    {
        if (job_max[i] == NULL)
        {
            return i;
        }
    }
    return -ENOSPC;
}

int rel_addjob(int fd1, int fd2)
{
    struct rel_job_st *me;
    // 分配rel_job_st内存
    me = malloc(sizeof(*me));

    int pos;
    // 初始化一次，启动线程轮训任务数组
    pthread_once(&thread_once, module_load);
    // 设置非阻塞
    me->fd1 = fd1;
    me->fd2 = fd2;

    me->job_state = STATE_RUNNING;

    me->fd1_save = fcntl(me->fd1, F_GETFL);
    fcntl(me->fd1, F_SETFL, me->fd1_save | O_NONBLOCK);
    me->fd2_save = fcntl(me->fd2, F_GETFL);
    fcntl(me->fd2, F_SETFL, me->fd2_save | O_NONBLOCK);

    // 初始化 rel_job_st
    me->fsm12.state = STATE_R;
    me->fsm12.sfd = me->fd1;
    me->fsm12.dfd = me->fd2;

    me->fsm21.state = STATE_R;
    me->fsm21.sfd = me->fd2;
    me->fsm21.dfd = me->fd1;
    

    // 加锁数组job寻找位置
    pthread_mutex_lock(&mutex_job);
    
    pos = get_free_pos_unlock();
    
    if (pos < 0)
    {
        // 恢复状态，释放资源和锁
        fcntl(me->fd1, F_SETFL, me->fd1_save);
        fcntl(me->fd2, F_SETFL, me->fd2_save);
        free(me);
        pthread_mutex_unlock(&mutex_job);
        return -ENOSPC;
    }
    // pos位置赋值
    job_max[pos] = me;
    pthread_mutex_unlock(&mutex_job);

    return pos;
}

int rel_canceljob(int id)
{
}

int rel_waitjob(int id, struct rel_state_st *p)
{
}

int rel_statjob(int id, struct rel_state_st *p)
{
}
