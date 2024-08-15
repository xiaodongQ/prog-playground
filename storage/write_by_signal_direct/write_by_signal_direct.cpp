#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/sysmacros.h>

#define FILE_PATH "./testfile"

void signal_handler(int sig)
{
    struct stat st;
    int fd;
    off_t offset;
    size_t block_size;

    if (sig == SIGUSR1) {
        printf("Received SIGUSR1 signal, writing to file...\n");

        // 打开文件并获取文件系统块大小
        fd = open(FILE_PATH, O_RDWR | O_CREAT | O_DIRECT, 0644);
        if (fd == -1) {
            perror("open");
            exit(1);
        }

        if (fstat(fd, &st) == -1) {
            perror("fstat");
            close(fd);
            exit(1);
        }

        // 确定文件系统块大小
        block_size = (size_t)st.st_blksize;
        if (block_size == 0) {
            // 如果 st_blksize 为 0，则使用 getconf 或默认值
            block_size = 4096; // 默认假设为 4K
        }

        // 确保文件大小至少为一个块大小
        if (st.st_size < block_size) {
            off_t new_size = block_size;
            if (ftruncate(fd, new_size) == -1) {
                perror("ftruncate");
                close(fd);
                exit(1);
            }
        }

        // 将文件指针移到对齐的偏移量
        offset = lseek(fd, 0, SEEK_SET);
        if (offset % block_size != 0) {
            offset += (block_size - (offset % block_size));
            if (lseek(fd, offset, SEEK_SET) == -1) {
                perror("lseek");
                close(fd);
                exit(1);
            }
        }

        // 创建足够大的缓冲区以满足对齐要求
        char *buf = (char*)malloc(block_size);
        if (!buf) {
            perror("malloc");
            close(fd);
            exit(1);
        }

        memset(buf, 'a', block_size - 1); // 填充数据
        buf[block_size - 1] = '\0'; // 终止符

        ssize_t bytes_written = write(fd, buf, block_size);
        if (bytes_written != (ssize_t)block_size) {
            if (errno == EINVAL) {
                printf("write failed with EINVAL. This usually means the buffer is not properly aligned or the file pointer is not at the right position.\n");
            }
            perror("write");
            free(buf);
            close(fd);
            exit(1);
        }
        printf("write success\n");
        free(buf);
        close(fd);
    }
}

int main()
{
    pid_t pid = getpid();
    printf("pid:%d\n", pid);

    // 安装信号处理器
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    while (1) {
        // 无操作，等待信号
        pause();
    }

    return 0;
}
