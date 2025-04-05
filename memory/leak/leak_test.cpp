#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#ifdef TEST_LEAK
#include <sanitizer/lsan_interface.h>
#endif

#define MAX_CHUNK_SIZE 1024 * 1024 // 最大块大小为 1 MB
#define MIN_CHUNK_SIZE 1024        // 最小块大小为 1 KB

// 模拟随机内存泄漏的函数
void* random_leak_memory(void* arg) {
    int max_chunks = *(int*)arg;
    srand(time(NULL)); // 初始化随机数种子

    for (int i = 0; i < max_chunks; i++) {
        // 随机生成内存块大小 (1 KB 到 1 MB)
        size_t chunk_size = (rand() % (MAX_CHUNK_SIZE - MIN_CHUNK_SIZE + 1)) + MIN_CHUNK_SIZE;

        // 分配内存
        void *ptr = malloc(chunk_size);
        if (ptr == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        // 填充数据以确保内存真正被使用
        memset(ptr, 0, chunk_size);
        printf("Allocated chunk %d of size %zu bytes\n", i + 1, chunk_size);

        // 随机决定是否释放内存（50% 的概率不释放）
        if (rand() % 2 == 0) {
            printf("Freeing chunk %d\n", i + 1);
            free(ptr);
        } else {
            printf("Leaking chunk %d\n", i + 1);
        }

        // 等待模拟实际运行中的内存使用
        sleep(1);
    }

    return NULL;
}

// 模拟空悬指针的问题
void simulate_dangling_pointer() {
    // 分配内存并初始化
    int *ptr = (int*)malloc(sizeof(int));
    if (ptr == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    *ptr = 42;
    printf("Allocated memory and initialized with value: %d\n", *ptr);

    // 释放内存
    free(ptr);
    printf("Memory freed, but ptr is still accessible.\n");

    // 访问已释放的内存（产生空悬指针）
    printf("Dangling pointer triggered: Accessing freed memory...\n");
    // 这里尝试访问已经释放的内存
    printf("Value at dangling pointer: %d\n", *ptr); // 可能导致未定义行为
}

// 模拟野指针的问题
void simulate_wild_pointer() {
    int *wild_ptr; // 声明但不初始化
    printf("Wild pointer declared but not initialized.\n");

    // 尝试使用未初始化的指针（产生野指针）
    printf("Wild pointer triggered: Accessing uninitialized memory...\n");
    // 这里尝试访问未初始化的指针
    printf("Value at wild pointer: %d\n", *wild_ptr); // 可能导致段错误
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <max_number_of_chunks>\n", argv[0]);
        return EXIT_FAILURE;
    }
    printf("ASAN_OPTIONS=%s\n", getenv("ASAN_OPTIONS"));
    printf("LSAN_OPTIONS=%s\n", getenv("LSAN_OPTIONS"));

    int max_chunks = atoi(argv[1]);

    if (max_chunks <= 0) {
        fprintf(stderr, "Please provide a positive number of chunks.\n");
        return EXIT_FAILURE;
    }

    pthread_t leak_thread;
    pthread_create(&leak_thread, NULL, random_leak_memory, &max_chunks);

    // 主线程进行空悬指针和野指针的模拟
    printf("\nTesting Dangling Pointer:\n");
    simulate_dangling_pointer();

    printf("\nTesting Wild Pointer:\n");
    simulate_wild_pointer();

    printf("\nRandom memory issue simulation completed. Check memory usage with tools like Valgrind or memleak.\n");

    // 等待泄漏线程完成
    pthread_join(leak_thread, NULL);
    printf("\nAll Tests Done.\n");

    // 手动触发内存泄漏检查，避免手动ctrl+c打断下面的while循环时无法触发检查
#ifdef TEST_LEAK
    __lsan_do_leak_check();
#endif

    // 主动进入无限循环，方便观察内存占用情况
    while (1) {
        sleep(1);
    }

    return 0;
}
