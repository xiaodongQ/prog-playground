#include <iostream>  
#include <sys/socket.h>  
#include <sys/types.h>
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <unistd.h>  
#include <string.h>  
  
const int PORT = 8080;  
const int BACKLOG = 5;
  
int main() {  
    int server_fd, new_socket;  
    struct sockaddr_in address;  
  
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
  
    // 您可以添加代码来让服务器持续运行，例如：  
	while (true) {  
		struct sockaddr_in client_address; // 用于存储客户端地址信息
		socklen_t client_len = sizeof(client_address);

		// 使用accept函数接受连接请求
		if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_len)) < 0) {
			perror("accept");
			continue; // 如果接受失败，继续下一次循环尝试
		}

		char client_ip[INET_ADDRSTRLEN]; // 用于存储客户端IP的字符串形式
		inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);

		std::cout << "Connection accepted from " << client_ip << ":" << ntohs(client_address.sin_port) << std::endl;

		// 读取数据逻辑
        char buffer[1024] = {0}; // 缓冲区用于存放接收的数据
        int valread;

        while ((valread = recv(new_socket, buffer, 1024, 0)) > 0) { // 循环读取直到没有更多数据
            std::cout << "Client: " << buffer << std::endl; // 打印接收到的数据
            memset(buffer, 0, 1024); // 清空缓冲区以便下一次读取
        }

        if (valread == 0) {
            std::cout << "Client disconnected" << std::endl; // 客户端正常关闭连接
        } else if (valread == -1) {
            perror("recv failed"); // 读取错误处理
        }

		// 不做关闭
    }  
  
    return 0;  
}
