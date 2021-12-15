#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mysem.h"

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT-LEFT+1)
#define N 4

static mysem_t* sem = NULL;

static void* thr_prime(void* p);

int main(){

    int i, err;
    pthread_t tid[THRNUM];
    

    // 初始化sem
    sem = mysem_init(N);
    

    // 创建线程，执行计算
    for(i = LEFT; i<=RIGHT; i++){
        mysem_sub(sem, 1);
        err = pthread_create(tid+(i-LEFT), NULL, thr_prime, (void*)i);
        if (err){
            fprintf(stderr, "pthread_create failed %s", strerror(err));
            exit(1);
        }
    }

    // 回收线程
    for(i = LEFT; i<=RIGHT; i++){
        pthread_join(tid[i-LEFT], NULL);
    }

    // 销毁sem
    mysem_destroy(sem);

    exit(0);
}

static void* thr_prime(void* p){
    int mark = 1;
    int i = (int) p;
    for(int j = 2; j < i/2; j++){
        if (i % j == 0){
            mark = 0;
        }
    }

    if (mark){
        printf("[%d]%d is a primer\n", (int)p, i);
        sleep(10);
    }

    mysem_add(sem, 1);

    pthread_exit(NULL);

}