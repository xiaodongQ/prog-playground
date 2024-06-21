#include <iostream>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <unistd.h>  
#include <string.h>  
  
const int PORT = 8080;  
const int BACKLOG = 5;
  
int main() {  
    int server_fd, new_socket;  
    struct sockaddr_in address;  
    int addrlen = sizeof(address);  
  
    // 创建socket  
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
  
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT);  
  
    // 绑定  
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
  
    // 监听  
    if (listen(server_fd, BACKLOG) < 0) {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
  
    std::cout << "Server listening on port " << PORT << std::endl;  
  
    // 注意：服务器不会接受连接，而是保持监听状态  
  
    // 您可以添加代码来让服务器持续运行，例如：  
    while (true) {  
        sleep(1); // 模拟服务器持续运行  
    }  
  
    return 0;  
}
