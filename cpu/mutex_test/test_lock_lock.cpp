#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mtx;
int a=0;

void* thread_proc(void*)
{
    printf("enter proc...\n");
    while(true) {
        for (int j = 0; j < 1000000; j++) {
            if (a != 0) {
                printf("a:%d, break\n", a);
                break;
            }
        }
        printf("mark11111.\n");

        if (a == 0) {
            // 线程外持锁了，此处lock会阻塞，直到外面通知unlock，后续就正常处理了
            pthread_mutex_lock(&mtx);
            pthread_mutex_unlock(&mtx);
            printf("continue...\n");
            continue;
        }
        printf("process...\n");
    }
}

int main(int argc, char *argv[])
{
    pthread_t tid;
    pthread_mutex_init(&mtx, NULL);
    a = 0;
    // 持锁。创建线程后，线程中再lock的话就会阻塞，直到下面通知unlock
    pthread_mutex_lock(&mtx);
    pthread_create(&tid, NULL, thread_proc, NULL);
    printf("create thread success\n");

    int c = 0;
    while(true) {
        sleep(1);
        if (c++ > 5) {
            // 解锁
            pthread_mutex_unlock(&mtx);
            c = 0;
        }
    }
}
