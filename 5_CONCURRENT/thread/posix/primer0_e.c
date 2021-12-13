#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT-LEFT+1)

struct thr_arg_st {
    int n;
};

static void* thr_prime(void* p){
    int mark = 1;
    int i = ((struct thr_arg_st *)p)->n;
    for (int j = 2; j < i/2; j++){
        if (i % j == 0){
            mark = 0;
            break;
        }
    }

    if (mark) {
        printf("%d is a primer\n", i);
    }

    pthread_exit(p);
}

int main(){
    void* ptr;
    int i, err;
    struct thr_arg_st *p;
    pthread_t tid[THRNUM];
    for (i = LEFT; i <= RIGHT; i++){
        //
        p = malloc(sizeof(p));
        p->n = i;
        err = pthread_create(tid+(i-LEFT), NULL, thr_prime, p);
        if (err){
            fprintf(stderr, "pthread_create():%s", strerror(err));
            exit(1);
        }
    }

    for (i = LEFT; i <= RIGHT; i++){
        pthread_join(tid[i-LEFT], &ptr);
        free(ptr);
    }

    exit(0);

}