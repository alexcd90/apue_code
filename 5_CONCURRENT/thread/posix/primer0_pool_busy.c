#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM 4

static int num = 0;
static pthread_mutex_t mutex_num = PTHREAD_MUTEX_INITIALIZER;

static void* thr_prime(void* p){
    int mark = 1, i;
    while(1){
        pthread_mutex_lock(&mutex_num);
        while(num==0){
            pthread_mutex_unlock(&mutex_num);
            sched_yield();
            pthread_mutex_lock(&mutex_num);
        }
        if (num == -1){
            pthread_mutex_unlock(&mutex_num);
            break;
        }

        i = num;
        num = 0;
        pthread_mutex_unlock(&mutex_num);

        for(int j = 2; j <i/2;j++){
            if (i%j == 0){
                mark = 0;
                break;
            }
        }
        if (mark){
            printf("[%d][%d] is a primer\n", (int)p, i);
        }
        mark = 1;
    }

    pthread_exit(NULL);
}

int main(){
    int i, err;
    pthread_t tid[THRNUM];
    
    // 启动线程
    for(i=0; i<=THRNUM; i++){
        err = pthread_create(tid+i, NULL, thr_prime, (void*) i);
        if (err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
            exit(1);
        }
    }


    // 分配任务
    for(i = LEFT; i<=RIGHT;i++){
        pthread_mutex_lock(&mutex_num);
        while(num!=0){
            pthread_mutex_unlock(&mutex_num);
            sched_yield();
            pthread_mutex_lock(&mutex_num);
        }
        num = i;
        pthread_mutex_unlock(&mutex_num);
    }

    // 设置结束标示
    pthread_mutex_lock(&mutex_num);
    while(num!=0){
        pthread_mutex_unlock(&mutex_num);
        sched_yield();
        pthread_mutex_lock(&mutex_num);
    }
    num = -1;
    pthread_mutex_unlock(&mutex_num);


    for(i = 0; i <= THRNUM;i++){
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mutex_num);
    exit(0);

}