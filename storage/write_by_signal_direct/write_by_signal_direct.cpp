#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define FILENAME "/home/tempfile"
#define BUFFER_SIZE (4096 * 2) // 假设文件系统块大小为 4KB，这里写8KB数据
#define BLOCK_SIZE 4096 // 文件系统块大小，通常是 4KB，用于下面的对齐

int fd;
char *buffer;
off_t offset;

void handle_signal(int signum)
{
	if (signum != SIGUSR1) {
		printf("signum: %d not expected!\n", signum);
		exit(1);
	}
    // 写入数据
    if (write(fd, buffer + offset, BUFFER_SIZE - offset) != (BUFFER_SIZE - offset)) {
        perror("Error writing to file");
        exit(1);
    }

    // 更新偏移量
    offset += (BUFFER_SIZE - offset);

    // 如果文件写满了，重置偏移量
    if (offset >= BUFFER_SIZE) {
        offset = 0;
    }

    // printf("Wrote %zu bytes to file.\n", BUFFER_SIZE - offset);
}

int main(void)
{
    pid_t pid = getpid();
    printf("pid:%d\n", pid);

    void *ptr;

    // 初始化偏移量
    offset = 0;

    // 打开文件，使用 O_DIRECT 标志
    fd = open(FILENAME, O_WRONLY | O_CREAT | O_DIRECT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // 分配缓冲区，并确保它是在页面边界上对齐
    if (posix_memalign(&ptr, BLOCK_SIZE, BUFFER_SIZE) != 0) {
        perror("Memory allocation failed");
        close(fd);
        return 1;
    }

    buffer = (char *)ptr;

    // 填充缓冲区
    memset(buffer, 'X', BUFFER_SIZE);

    // 设置信号处理函数
    signal(SIGUSR1, handle_signal);

    // 主循环
    while (1) {
        pause();
    }

    // 不会执行到这里，因为程序是无限循环
    free(buffer);
    close(fd);
    return 0;
}
