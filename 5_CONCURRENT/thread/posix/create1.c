#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static void* func(void* p){
    puts("Thread is working");
    // return NULL
    pthread_exit(NULL);
}

int main(){
    pthread_t tid;
    int err;

    puts("Begin");
    err = pthread_create(&tid, NULL, func, NULL); // 不设置属性
    if (err) {
        fprintf(stderr, "pthread_create failed %s \n", strerror(err));
        exit(1);
    }

    // tid = 0
    err = pthread_join(tid, NULL);
    if (err) {
        fprintf(stderr, "pthread_join failed %s \n", strerror(err));
    }
    puts("End!");

    exit(0);
}