#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200

int main(){
    int i, j, mark;
    pid_t pid;

    for (i = LEFT; i <= RIGHT; i++) {
        pid = fork();

        if (pid < 0){
            perror("fork()");
            exit(1);
        }

        if (pid == 0){
            mark = 1;
            for (j = 2; j < i/2; j++){
                if (i % j == 0){
                    mark = 0;
                }
            }

            if (mark){
                printf("%d is a primer\n", i);
            }

            sleep(10); //父进程结束，子进程变孤儿！

            exit(0); // 这里必须结束子进程，否则会创建大量进程
        }
    }

    sleep(60); //父进程未结束，子进程变僵尸

    exit(0);
}