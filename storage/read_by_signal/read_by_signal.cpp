#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

// 信号处理器函数
void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        // 打开 /etc/fstab 文件
        int fd = open("/etc/fstab", O_RDONLY);
        if (fd == -1) {
            perror("Error opening file");
            return;
        }

        char buffer[1024];
        ssize_t bytes_read;

        // 读取文件内容
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, bytes_read);  // 输出到标准输出
        }

        if (bytes_read == -1) {
            perror("Error reading file");
        }

        close(fd);  // 关闭文件描述符
    }
}

int main() {
    pid_t pid = getpid();

    // 打印进程ID
    printf("Process ID: %d\n", pid);

    // 设置信号处理器
    signal(SIGUSR1, signal_handler);

    // 进入无限循环等待信号
    for (;;) {
        pause();
    }

    return 0;
}
