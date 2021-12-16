#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// thread func

void* func(void* p){
    sleep(100);
    pthread_exit(NULL);
}


int main(){

    // 基础参数
    long long i = 0;
    int err;
    pthread_t tid;
    pthread_attr_t attr;
    size_t size;

    // 设置线程栈大小
    pthread_attr_init(&attr);
    pthread_attr_getstacksize(&attr, &size);
    printf("before stack size: [%lld]\n", size);

    pthread_attr_setstacksize(&attr, 1024);
    pthread_attr_getstacksize(&attr, &size);
    printf("after stack size: [%lld]\n", size);

    // 无限循环创建线程
    for(; ; i++){
        err = pthread_create(&tid, &attr, func, NULL);
        if (err){
            fprintf(stderr, "pthread_create():%s\n", strerror(err));
        }
    }

    // 回收线程，销毁线程属性
    printf("max thread limit : [%lld]\n", i);
    pthread_join(tid, NULL);
    pthread_attr_destroy(&attr);

    exit(0);
}