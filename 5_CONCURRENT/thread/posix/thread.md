# APUE  并发（信号、线程）

## 二、线程

### 1.线程的概念

一个正在运行的函数

创建出来的线程都是兄弟

用到的库应该默认支持多线程并发!

posix线程是一套标准,而不是实现

openmp线程标准

线程标识:pthread_t 可能是整形,可能是结构体

ps axm

ps -a -L

信号和多线程小范围混用合适

pthread_equal();

```c
SYNOPSIS
        #include <pthread.h>

        Compile and link with -pthread.

        int pthread_equal(pthread_t t1, pthread_t t2);
```

pthread_self();

```c
NAME
       pthread_self - obtain ID of the calling thread

SYNOPSIS
        #include <pthread.h>

        Compile and link with -pthread.

        pthread_t pthread_self(void);
```

### 2.线程的创建

```c
NAME
       pthread_create - create a new thread

SYNOPSIS
        #include <pthread.h>

        int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                      void *(*start_routine) (void *), void *arg);

        Compile and link with -pthread.


1.thread 回填一个线程标识

2.attr指定属性 默认情况属性 **可以解决80%的问题**

3.void *(*start_routine) (void *) 这个是兄弟线程的函数 void* * 可以传各类地址

RETURN VALUE

    On success, pthread_create() returns 0; on error, it returns an **error number**, and the contents of *thread are undefined.

只能用strnumber来区分错误信号

```

线程的调度取决于调度器策略,一开始create1 打印不出线程中的内容?  线程还没来的及调度,进程就结束了exit(0),所以打印不出线程中的内容! 我自己加上pause();就可以看到了.

###  3.线程的终止

3种方式: 
        
    1.线程从启动例程中返回,返回就是线程的退出码

    2.线程可以被同一进程中的其他线程取消  //算是异常终止的一种方式
   
    3.线程调用pthread_exit()函数,相当于进程阶段的exit

pthread_exit();

```c
NAME
       pthread_exit - terminate calling thread

SYNOPSIS
       #include <pthread.h>

       void pthread_exit(void *retval);
    
       Compile and link with -pthread.

```

pthread_join(); //进程的收尸

```c
NAME
       pthread_join - join with a terminated thread

SYNOPSIS
       #include <pthread.h>

        int pthread_join(pthread_t thread, void **retval);

        Compile and link with -pthread.

**retval  可以给它一个一级指针的地址  

RETURN VALUE
       On success, pthread_join() returns 0; on error, it  returns  an
       error number

```

### 4.线程栈的清理

pthread_cleanup_push(); //挂钩子函数 

pthread_cleanup_pop()  //取钩子函数

```c
NAME
       pthread_cleanup_push, pthread_cleanup_pop - push and pop thread
       cancellation clean-up handlers

SYNOPSIS
       #include <pthread.h>

       void pthread_cleanup_push(void (*routine)(void *),
                                 void *arg);
       void pthread_cleanup_pop(int execute);
    
       Compile and link with -pthread.

```
上述是个宏, 可以用gcc cleanup.c -E 来查看

有多少个push,就有多少个pop

### 5.线程的取消

一定会用到线程的取消

pthread_cancel();
```c

NAME
       pthread_cancel - send a cancellation request to a thread

SYNOPSIS
       #include <pthread.h>

       int pthread_cancel(pthread_t thread);
    
       Compile and link with -pthread.
```

取消有两种状态:允许和不允许.

如果设置允许取消又分为:异步取消cancel和推迟取消cancel(默认) ->推迟至cancel点再响应

cancel点:POSIX定义的cancel点,都是可能引发阻塞的系统调用

```c
NAME
       pthread_setcancelstate, pthread_setcanceltype - set cancelability state and type

SYNOPSIS
        #include <pthread.h>


        int pthread_setcancelstate(int state, int *oldstate); //设置是否允许取消
        int pthread_setcanceltype(int type, int *oldtype);  //设置取消方式

        Compile and link with -pthread.
```

pthread_testcancel();

```C
NAME
       pthread_testcancel  - request delivery of any pending cancellation request

SYNOPSIS


   #include <pthread.h>
   void pthread_testcancel(void); //什么都不做,就是一个取消点
```



### 6.线程分离

pthread_detach();
```c

NAME
       pthread_detach - detach a thread

SYNOPSIS

        #include <pthread.h>   //已经被detach的线程不能用join回收
        int pthread_detach(pthread_t thread);
```

静态的代码是共用的,

线程的栈是独立的

栈的

stack size              (kbytes, -s) 8192  //可以开多少个线程


### 7.线程同步

互斥量

pthread_mutex_t

pthread_mutex_init

pthread_mutex_destroy

pthread_mutex_lock

pthread_mutex_trylock

pthread_mutex_unlock

```c

NAME
       pthread_mutex_init — destroy and initialize a mutex 这个是

SYNOPSIS
       #include <pthread.h>


       int pthread_mutex_destroy(pthread_mutex_t *mutex);
       int pthread_mutex_init(pthread_mutex_t *restrict mutex,
           const pthread_mutexattr_t *restrict attr);
       pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

```

pthread_mutex_lock();

```c
NAME
       pthread_mutex_lock, pthread_mutex_trylock, pthread_mutex_unlock
       — lock and unlock a mutex

SYNOPSIS
        #include <pthread.h>


        int pthread_mutex_lock(pthread_mutex_t *mutex);    //阻塞 死等
        int pthread_mutex_trylock(pthread_mutex_t *mutex);  //非阻塞
        int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

阻塞调用是指调用结果返回之前，当前线程会被挂起。调用线程只有在得到结果之后才会返回。
非阻塞调用指在不能立刻得到结果之前，该调用不会阻塞当前线程。

**临界区的任何一个跳转到临界区外的语句,都需要解锁再跳转!!!!**

**临界区中的函数跳转也是一个问题,如果函数跳转回不来会产生死锁!!!**

sched_yield() 出让调度器

pthread_once(); 实现某个模块的单次初始化只被调用一次

```C
NAME
       pthread_once — dynamic package initialization

SYNOPSIS
       #include <pthread.h>
   int pthread_once(pthread_once_t *once_control,
       void (*init_routine)(void));
   pthread_once_t once_control = PTHREAD_ONCE_INIT;
```

### 8.条件变量:

pthread_cond_t

pthread_cond_init();

pthread_cond_destroy();

```c

NAME
       pthread_cond_destroy, pthread_cond_init — destroy and  initial‐
       ize condition variables

SYNOPSIS
       #include <pthread.h>

       int pthread_cond_destroy(pthread_cond_t *cond);
       int pthread_cond_init(pthread_cond_t *restrict cond,
           const pthread_condattr_t *restrict attr);
       pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


```

### 9.线程相关属性

线程同步的属性

### 10.重入

### 11.线程与信号的关系

线程于fork