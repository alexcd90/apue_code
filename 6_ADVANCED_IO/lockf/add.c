#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROCNUM 5
#define FILENAME "/tmp/out"
#define LINESIZE 1024

static void func_add()
{
    FILE *fp;
    int fd;
    char buf[LINESIZE];

    // fopen按照r+方式打开
    fp = fopen(FILENAME, "r+");
    if (fp == NULL)
    {
        perror("fopen()");
        exit(1);
    }
    // fileno获取fd
    fd = fileno(fp);
    // lockf加锁释放文件锁
    lockf(fd, F_LOCK, 0);
    /****        临界区       ******/
    // fgets读取内容
    fgets(buf, LINESIZE, fp);
    // fseek调整文件指针到文件首位
    fseek(fp, 0, SEEK_SET);
    //fprintf加一后，写入文件
    fprintf(fp, "%d\n", atoi(buf) + 1);
    // fflush刷新缓冲区
    fflush(fp);
    /****        临界区       ******/
    // 释放文件锁
    lockf(fd, F_ULOCK, 0);

    // 关闭文件
    close(fd);
}

int main()
{

    pid_t pid;

    // 循环创建子进程
    for (int i = 0; i < PROCNUM; i++)
    {
        pid = fork();

        if (pid < 0)
        {
            perror("fork()");
            exit(1);
        }

        if (pid == 0)
        {
            // 读取文件进行加运算
            func_add();
            exit(0);
        }
    }

    // 循环回收子进程
    for (int i = 0; i < PROCNUM; i++)
    {
        wait(NULL);
    }

    exit(0);
}