#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/epoll.h>
#define TTY1 "/dev/tty11"
#define TTY2 "/dev/tty12"
#define BUFSIZE 1024

enum
{
    STATE_R = 1,
    STATE_W,
    STATE_AUTO,
    STATE_Ex,
    STATE_T
};

struct fsm_st
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *errstr;
};

static void fsm_driver(struct fsm_st *fsm)
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

static int max(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

static void relay(int fd1, int fd2)
{

    // fd1, fd2 设置非阻塞
    int fd1_save, fd2_save;
    struct fsm_st fsm12, fsm21;

    fd1_save = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, fd1_save | O_NONBLOCK);
    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save | O_NONBLOCK);

    // fsm12 fsm21结构体初始化
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;
    fsm12.state = STATE_R;

    fsm21.sfd = fd2;
    fsm21.dfd = fd1;
    fsm21.state = STATE_R;

    // 创建epoll实例
    int efd;
    struct epoll_event ev;

    efd = epoll_create(2);
    if (efd < 0)
    {
        perror("epoll_create() failed.");
        exit(1);
    }

    // epoll_ctl设置epoll_event
    ev.data.fd = fd1;
    ev.events = 0;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd1, &ev);

    ev.data.fd = fd2;
    ev.events = 0;
    ev.events = 0;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd2, &ev);

    // 非终止状态无限循环状态集驱动
    while (fsm12.state != STATE_T || fsm21.state != STATE_T)
    {

        ev.data.fd = fd1;
        ev.events = 0;
        // 根据状态集运行状态注册监听事件
        if (fsm12.state == STATE_R)
        {
            ev.events |= EPOLLIN;
        }

        if (fsm21.state == STATE_W)
        {
            ev.events |= EPOLLOUT;
        }
        epoll_ctl(efd, EPOLL_CTL_MOD, fd1, &ev);

        ev.data.fd = fd2;
        ev.events = 0;

        if (fsm21.state == STATE_R)
        {
            ev.events |= EPOLLIN;
        }

        if (fsm12.state == STATE_W)
        {
            ev.events |= EPOLLOUT;
        }

        epoll_ctl(efd, EPOLL_CTL_MOD, fd2, &ev);

        // 状态小于auto的进入epoll_wait
        if (fsm12.state < STATE_AUTO || fsm21.state < STATE_AUTO)
        {
            // epoll_wait轮询事件集合
            if (epoll_wait(efd, &ev, 1, -1) < 0)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                perror("epoll_wait()");
                exit(1);
            }
            // 根据事件集合推动状态
            if ((ev.data.fd == fd1 && ev.events & EPOLLIN) || (ev.data.fd == fd2 && ev.events & EPOLLOUT) || fsm12.state > STATE_AUTO)
            {
                fsm_driver(&fsm12);
            }
            if ((ev.data.fd == fd2 && ev.events & EPOLLIN) || (ev.data.fd == fd1 && ev.events & EPOLLOUT) || fsm21.state > STATE_AUTO)
            {
                fsm_driver(&fsm21);
            }
        }
    }

    // 回复fcntl原始状态
    fcntl(fd1, F_SETFL, fd1_save);
    fcntl(fd2, F_SETFL, fd2_save);
}

int main()
{

    int sfd, dfd;
    // 打开两个tty终端
    sfd = open(TTY1, O_RDWR);
    if (sfd < 0)
    {
        perror("open()");
        exit(1);
    }
    write(sfd, "TTY1\n", 5);

    dfd = open(TTY2, O_RDWR);
    if (dfd < 0)
    {
        perror("open()");
        exit(1);
    }

    write(dfd, "TTY2\n", 5);
    // 进行数据中继
    relay(sfd, dfd);

    // 关闭两个终端
    close(dfd);
    close(sfd);

    exit(0);
}