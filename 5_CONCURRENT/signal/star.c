#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void int_handler(int s) {
    write(1, "!!!", 3);
}

int main(){


    // signal(SIGINT, SIG_IGN); // SIG_IGN 打断信号
    signal(SIGINT, int_handler);

    for(int i = 0; i< 10; i++){
        write(1, "*", 1);
        sleep(1);
    }

    exit(0);
}