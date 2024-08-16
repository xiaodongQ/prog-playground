#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define FILE_PATH "/home/tempfile"

void signal_handler(int sig)
{
    if (sig == SIGUSR1) {
        // printf("Received SIGUSR1 signal, writing to file...\n");
        FILE *fp = fopen(FILE_PATH, "w");
        if (fp == NULL) {
            perror("fopen");
            exit(1);
        }
        if (fprintf(fp, "hello world\n") < 0) {
            printf("write failed!\n");
            perror("fprintf");
        }
        fclose(fp);
    }
}

int main()
{
    pid_t pid = getpid();
    printf("pid:%d\n", pid);

    // Install signal handler for SIGUSR1
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    while (1) {
        // Do nothing, just wait for signal
        pause();
    }

    return 0;
}
