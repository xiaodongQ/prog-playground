#include <stdio.h>  
#include <stdlib.h>  
#include <sys/time.h>  
#include <time.h>  
#include <sched.h>  
#include <sys/types.h>  
#include <unistd.h>      //pipe()  
  
int main()  
{  
    int x, i, fd[2], p[2];  
    char send    = 's';  
    char receive;  
    pipe(fd);  
    pipe(p);  
    struct timeval tv;  
    struct sched_param param;  
    param.sched_priority = 0;  
  
    // 说明：父进程和子进程之间的通信循环次数为 10000 次，
    // 在每次循环中，父进程和子进程之间会进行一次完整的通信，
    // 即父进程写入、子进程读取、子进程写入、父进程读取，
    // 这个过程中会发生 2 次上下文切换（父进程到子进程，子进程到父进程）
    // 总的上下文切换次数为 20000 次
    while ((x = fork()) == -1); 
    if (x==0) {  
        // 子进程
        sched_setscheduler(getpid(), SCHED_FIFO, &param);  
        gettimeofday(&tv, NULL);  
        printf("Before Context Switch Time %u s, %u us\n", tv.tv_sec, tv.tv_usec);  
        for (i = 0; i < 10000; i++) {  
            read(fd[0], &receive, 1);  
            write(p[1], &send, 1);  
        }  
        exit(0);  
    }  
    else {  
        // 父进程
        sched_setscheduler(getpid(), SCHED_FIFO, &param);  
        for (i = 0; i < 10000; i++) {  
            write(fd[1], &send, 1);  
            read(p[0], &receive, 1);  
        }  
        gettimeofday(&tv, NULL);  
        printf("After Context SWitch Time %u s, %u us\n", tv.tv_sec, tv.tv_usec);  
    }  
    return 0;  
} 
