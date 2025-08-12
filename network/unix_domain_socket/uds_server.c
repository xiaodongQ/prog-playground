#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define UDS_PATH "./uds_demo.sock"  // UDS的路径
#define BUF_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_len;
    char buf[BUF_SIZE];

    // 1. 删除已存在的UDS文件（避免绑定失败）
    unlink(UDS_PATH);

    // 2. 创建UDS（流式）
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket create failed");
        exit(EXIT_FAILURE);
    }

    // 3. 初始化服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, UDS_PATH, sizeof(server_addr.sun_path) - 1);  // 注意路径长度

    // 4. 绑定地址到socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. 监听连接（最多允许5个等待连接）
    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on %s...\n", UDS_PATH);

    // 6. 接受客户端连接
    client_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");

    // 7. 读取客户端消息
    ssize_t n = read(client_fd, buf, BUF_SIZE);
    if (n == -1) {
        perror("read failed");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Received from client: %.*s\n", (int)n, buf);

    // 8. 回复客户端
    const char* reply = "Hello from server!";
    if (write(client_fd, reply, strlen(reply)) == -1) {
        perror("write failed");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 9. 关闭连接
    close(client_fd);
    close(server_fd);
    unlink(UDS_PATH);  // 清理UDS文件
    printf("Server closed.\n");

    return 0;
}
