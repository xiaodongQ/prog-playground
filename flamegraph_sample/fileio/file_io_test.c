#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define FILE_NAME "test_file.txt"
#define WRITE_SIZE_MB 50  // 写入文件的大小，单位为兆字节
#define BUFFER_SIZE 4096  // 缓冲区大小

// 生成随机缓冲区内容
void generate_random_buffer(char *buffer, size_t size) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < size - 1; i++) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buffer[size - 1] = '\0';
}

// 顺序写入文件
void sequential_write(const char *filename) {
    printf("Starting sequential write...\n");
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file for sequential write");
        return;
    }

    size_t total_bytes = WRITE_SIZE_MB * 1024 * 1024;
    size_t bytes_written = 0;
    char buffer[BUFFER_SIZE];

    while (bytes_written < total_bytes) {
        generate_random_buffer(buffer, BUFFER_SIZE);
        size_t write_size = (bytes_written + BUFFER_SIZE > total_bytes) ? total_bytes - bytes_written : BUFFER_SIZE;
        fwrite(buffer, sizeof(char), write_size, file);
        bytes_written += write_size;
    }

    fclose(file);
    printf("Sequential write completed.\n");
}

// 顺序读取文件
void sequential_read(const char *filename) {
    printf("Starting sequential read...\n");
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file for sequential read");
        return;
    }

    size_t total_bytes = WRITE_SIZE_MB * 1024 * 1024;
    size_t bytes_read = 0;
    char buffer[BUFFER_SIZE];

    while (bytes_read < total_bytes) {
        size_t read_size = (bytes_read + BUFFER_SIZE > total_bytes) ? total_bytes - bytes_read : BUFFER_SIZE;
        if (fread(buffer, sizeof(char), read_size, file) != read_size) {
            if (feof(file)) {
                break;
            } else {
                perror("Error reading file");
                break;
            }
        }
        bytes_read += read_size;
    }

    fclose(file);
    printf("Sequential read completed.\n");
}

// 随机写入文件
void random_write(const char *filename) {
    printf("Starting random write...\n");
    FILE *file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Failed to open file for random write");
        return;
    }

    size_t total_bytes = WRITE_SIZE_MB * 1024 * 1024;
    char buffer[BUFFER_SIZE];

    for (size_t i = 0; i < total_bytes / BUFFER_SIZE; i++) {
        long offset = rand() % (total_bytes - BUFFER_SIZE);
        fseek(file, offset, SEEK_SET);
        generate_random_buffer(buffer, BUFFER_SIZE);
        fwrite(buffer, sizeof(char), BUFFER_SIZE, file);
    }

    fclose(file);
    printf("Random write completed.\n");
}

// 随机读取文件
void random_read(const char *filename) {
    printf("Starting random read...\n");
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file for random read");
        return;
    }

    size_t total_bytes = WRITE_SIZE_MB * 1024 * 1024;
    char buffer[BUFFER_SIZE];

    for (size_t i = 0; i < total_bytes / BUFFER_SIZE; i++) {
        long offset = rand() % (total_bytes - BUFFER_SIZE);
        fseek(file, offset, SEEK_SET);
        if (fread(buffer, sizeof(char), BUFFER_SIZE, file) != BUFFER_SIZE) {
            if (feof(file)) {
                break;
            } else {
                perror("Error reading file");
                break;
            }
        }
    }

    fclose(file);
    printf("Random read completed.\n");
}

// 信号处理函数
void signal_handler(int signum) {
    if (signum == SIGUSR1) {
        srand(time(NULL));
        printf("File I/O test program started by signal.\n");

        // 顺序写入
        sequential_write(FILE_NAME);
        // 顺序读取
        sequential_read(FILE_NAME);
        // 随机写入
        random_write(FILE_NAME);
        // 随机读取
        random_read(FILE_NAME);

        printf("File I/O test program finished.\n");
    }
}

int main() {
    // 注册信号处理函数
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
        perror("Failed to register signal handler");
        return 1;
    }

    printf("Waiting for SIGUSR1 signal to start the test...\n");

    // 让程序进入休眠状态，等待信号
    while (1) {
        pause();
    }

    return 0;
}
