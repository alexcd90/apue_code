#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM 4

static int num = 0;
static pthread_mutex_t mutex_num = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_num = PTHREAD_COND_INITIALIZER;

static void* thr_prime(void* p){
    
    while(1){
        int mark = 1, i;

        pthread_mutex_lock(&mutex_num);
        while(num==0){
            pthread_cond_wait(&cond_num, &mutex_num);
        }
        if (num == -1){
            pthread_mutex_unlock(&mutex_num);
            break; //临界区的任何一个跳转到临界区外的语句,都需要解锁再跳转!!!!
        }

        i = num;
        num = 0;
        pthread_cond_broadcast(&cond_num);
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
            pthread_cond_wait(&cond_num, &mutex_num);
        }
        num = i;
        pthread_cond_signal(&cond_num);
        pthread_mutex_unlock(&mutex_num);
    }

    // 设置结束标示
    pthread_mutex_lock(&mutex_num);
    while(num!=0){
        pthread_cond_wait(&cond_num, &mutex_num);
    }
    num = -1;
    pthread_cond_broadcast(&cond_num);
    pthread_mutex_unlock(&mutex_num);


    for(i = 0; i <= THRNUM;i++){
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mutex_num);
    pthread_cond_destroy(&cond_num);
    exit(0);

}