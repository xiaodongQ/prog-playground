#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // 创建socket文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置socket选项，允许重复使用地址
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定socket到本地地址
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 开始监听，最大等待队列长度为5
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(true)
    {
        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            continue;
        }
        
        // 接收客户端消息，curl客户端请求时，需要先接收下其请求
        // valread = read(new_socket, buffer, 1024);
        // printf("Client: %s\n", buffer);

        // 用于打印客户端的IP地址和端口号
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));

        // 关闭连接
        printf("close, client_ip:%s, port:%d\n", client_ip, ntohs(address.sin_port));
        close(new_socket);
    }

    return 0;
}
