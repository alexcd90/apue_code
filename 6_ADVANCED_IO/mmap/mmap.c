#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
    int fd;
    struct stat statres;
    char *str;
    int count = 0;
    // main参数校验
    if (argc < 2)
    {
        fprintf(stderr, "Usage:...");
        exit(1);
    }

    // open打开一个只读文件
    fd = open(argv[1], O_RDONLY);

    // fstat统计一个文件大小
    if (fstat(fd, &statres) < 0)
    {
        perror("fstat()");
        exit(1);
    }

    // mmap映射一个文件，返回字符串首地址
    str = mmap(NULL, statres.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (str == MAP_FAILED)
    {
        perror("mmap() failed.");
        exit(1);
    }
    close(fd);
    // 循环判断字符是否是a
    for (int i = 0; i < statres.st_size; i++)
    {
        if (str[i] == 'a')
        {
            count++;
        }
    }
    printf("this file contain [%d] a.\n", count);
    // 解除映射
    munmap(str, statres.st_size);

    exit(0);
}