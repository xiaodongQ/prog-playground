#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define UDS_PATH "./uds_demo.sock"  // 与服务器相同的路径
#define BUF_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_un server_addr;
    char buf[BUF_SIZE];

    // 1. 创建UDS（流式）
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket create failed");
        exit(EXIT_FAILURE);
    }

    // 2. 初始化服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, UDS_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. 连接服务器
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connected to server.\n");

    // 4. 发送消息到服务器
    const char* msg = "Hello from client!";
    if (write(sock_fd, msg, strlen(msg)) == -1) {
        perror("write failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 5. 接收服务器回复
    ssize_t n = read(sock_fd, buf, BUF_SIZE);
    if (n == -1) {
        perror("read failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Received from server: %.*s\n", (int)n, buf);

    // 6. 关闭连接
    close(sock_fd);
    printf("Client closed.\n");

    return 0;
}
