# APUE  并发（信号、线程）

## 一、信号(第十章)

同步

异步

异步事件的处理：查询法（稠密的异步事件），通知法（稀疏的异步事件）

信号处理函数尽量小，小到一闪而过。

**信号处理函数中最好不要用标准IO可能刷新共同的缓冲区,最好使用系统调用!!**

### 1.信号的概念
信号是软件中断

信号的响应依赖于中断

### 2.singal()
段错误 修改unlimit -c 保存出错线程最大值，ulimit -c 10240 会生成core.1234

```c
    #include <signal.h>
    
    typedef void (*sighandler_t)(int);

    sighandler_t signal(int signum, sighandler_t handler);  //handler 处理程序
	    void(*signal(int signum, void (*func)(int)))(int);
```
**信号会打断阻塞的系统调用！**  例如start.c中的sleep()

### 3.信号的不可靠（行为的不可靠）
标准信号会丢失不是说明信号不可靠，信号行为不可靠，系统第一次调用没结束的时候可能发生的第二次调用

### 4.可重入函数
所有的系统调用都是可重入的，一部分库函数也是可以重入的

_r版本库函数是可重入

```c
    #include <time.h>

    char *asctime(const struct tm *tm);
    char *asctime_r(const struct tm *tm, char *buf);

    char *ctime(const time_t *timep);
    char *ctime_r(const time_t *timep, char *buf);

    struct tm *gmtime(const time_t *timep);
    struct tm *gmtime_r(const time_t *timep, struct tm *result);

    struct tm *localtime(const time_t *timep);
    struct tm *localtime_r(const time_t *timep, struct tm *result); //多一个自己给定的参数，空间有自己分配，结果不会被信号终端干扰
```

```c
    #include <string.h>

    void *memcpy(void *dest, const void *src, size_t n);

DESCRIPTION
       The  memcpy()  function  copies n bytes from memory area src to
       memory area dest.  The memory areas must not overlap.  Use mem‐
       move(3) if the memory areas do overlap.

```

### 5.信号的响应过程
其实进程和线程不分家，进程是人定的容器，现在我们的编程是单进程单线程。

信号从收到到响应有一个不可避免的延迟，中断不打断，看不到信号，**从kernel返回用户态** mask&pending

思考：信号如何忽略掉的？ 将mask某一位置0

 标准信号为什么要丢失？ 用位图，来一万个相同信号，最终都是置一个1

 标准信号的响应没有严格的顺序！先响应比较严重的信号

 不能从信号处理 函数中**随意**的往外跳！ 不能从setjmp跳出来（但是各家协定不一样），sigsetjmp，siglongjmp 会保存mask？

### 6.常用函数
kill();

//由于多半的信号是终止，这个函数是发送信号

```c
    #include <sys/types.h>
    #include <signal.h>

    int kill(pid_t pid, int sig);
```

raise();
```c
    #include <signal.h>

    int raise(int sig);
    //The  raise()  function sends a signal to the calling process or
    //thread.  In a single-threaded program it is equivalent to

	kill(getpid(), sig);

    //In a multithreaded program it is equivalent to

    pthread_kill(pthread_self(), sig);

```

alarm();
```c
    #include <unistd.h>

    unsigned int alarm(unsigned int seconds);
    //alarm()  arranges  for  a SIGALRM signal to be delivered to the
    //calling process in seconds seconds.
```

gcc -S xxxx.c 产生汇编文件

-O1 产生优化！

**volatile   //不要轻信内存中的值？ 去数值真正的空间取数值！**	

例：使用单一计时器构造一组函数，实现任意数量的计时器或者 	记得回去写anytimer.c!!

alarm 或 setitimer

setitimer();
```c
NAME
    get or set value of an interval timer

SYNOPSIS
    #include <sys/time.h>

    int getitimer(int which, struct itimerval *curr_value);
    int setitimer(int which, const struct itimerval *new_value,
                 struct itimerval *old_value);
```

pause();

abort(); //终止，并会保存一个core dump的出错现场！

```c
    NAME
        abort - cause abnormal process termination
    
    SYNOPSIS

        #include <stdlib.h>
        void abort(void);
```

system();
```c
NAME
       system - execute a shell command

SYNOPSIS
       #include <stdlib.h>

       int system(const char *command);

DESCRIPTION
       system() executes a command specified in command by calling /bin/sh -c command, and returns after the command has been completed.  During execution of the command, SIGCHLD
       will be blocked, and SIGINT and SIGQUIT will be ignored.
```

sleep(); -> alarm() + pause(); 源码里有sleep就很棒槌,要么测试忘删了

nanosleep(); 可以用于替换sleep

usleep();

select(); 超时设置，只传timeout，可以成为休眠！


**static volatile sig_atomic_t token = 0; //加上原子操作保证++和--是一气呵成的**

令牌桶 三个要素： CPS、BURST、TOKEN

永远不要觉得自己在写一个完整的函数，永远不要觉得自己在单枪匹马的工作！

### 7.信号集
信号集类型sigset_t
```c
    #include <signal.h>

    int sigemptyset(sigset_t *set);

    int sigfillset(sigset_t *set);

    int sigaddset(sigset_t *set, int signum);

    int sigdelset(sigset_t *set, int signum);

    int sigismember(const sigset_t *set, int signum);
```

### 8.信号屏蔽字/pending集合处理
sigprocmask();
```c
    #include <signal.h>

    /* Prototype for the glibc wrapper function */
    int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```

### 9.扩展
sigsuspend(); 见susp

显示的作用是等待一个信号，和pause很像！

```c
NAME
       sigsuspend, rt_sigsuspend - wait for a signal

SYNOPSIS
    #include <signal.h>
    
    int sigsuspend(const sigset_t *mask);
```

**sigaction();** 非常重要！！
```c
    struct sigaction {
           void     (*sa_handler)(int);
           void     (*sa_sigaction)(int, siginfo_t *, void *); //这个三参数的形式
           sigset_t   sa_mask;
           int        sa_flags;
           void     (*sa_restorer)(啊void);
       };
```

```c
    The siginfo_t data type  is  a  structure  with  the  following
    fields:

       siginfo_t {
           int      si_signo;     /* Signal number */
           int      si_errno;     /* An errno value */
           int      si_code;      /* Signal code */  ！！！这个很要用！！
           int      si_trapno;    /* Trap number that caused
                                     hardware-generated signal
                                     (unused on most architectures) 
                                     ////////////////...........
```
见mydeamon的用法！

while true; do kill -ALRM 219711; done //mytbf.c 没有识别信号来自哪里！

getcontext();

setcontext();

### 10.实时信号
实时信号需要排队

实时信号保存在哪个文件里?

susp_rt.c

**实时信号不丢失!!!!!**