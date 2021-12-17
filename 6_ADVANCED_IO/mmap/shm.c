
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MEMSIZE 1024

int main()
{

    char *ptr;
    pid_t pid;

    // mmap共享内存映射
    ptr = mmap(NULL, MEMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap()");
        exit(1);
    }
    // fork子进程
    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
        exit(1);
    }
    // 子进程往共享内存写，解除映射，退出子进程
    if (pid == 0)
    {
        strcpy(ptr, "HELLO!");
        munmap(ptr, MEMSIZE);
        exit(0);
    }
    // 父进程回收子进程，读取共享内存输出终端，解除映射
    else
    {
        wait(NULL);
        puts(ptr);
        munmap(ptr, MEMSIZE);
    }

    exit(0);
}