#include <iostream>  
#include <thread>  
#include <vector>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include <string.h>  
  
char SERVER_IP[64] = "";  
const int PORT = 8080;  
  
void send_message(int num_requests) {  
    for (int i = 0; i < num_requests; ++i) {  
        int sock = 0;  
        struct sockaddr_in serv_addr;  
  
        // 创建socket  
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
            std::cerr << "Socket creation error" << std::endl;  
            return;  
        }  
  
        serv_addr.sin_family = AF_INET;  
        serv_addr.sin_port = htons(PORT);  
  
        // 将服务器的IP地址转换为网络字节序  
        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {  
            std::cerr << "Invalid address/ Address not supported" << std::endl;  
            close(sock);  
            return;  
        }  
  
        // 连接到服务器  
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {  
            std::cerr << "Connection Failed" << std::endl;  
            close(sock);  
            return;  
        }  
  
        std::string message = "helloworld";  
        // 发送消息  
        if (send(sock, message.c_str(), message.length(), 0) < 0) {  
            std::cerr << "Send failed" << std::endl;  
        } else {  
            std::cout << "Message sent: " << message << std::endl;  
        }  
  
        close(sock);  
    }  
}  
  
int main(int argc, char *argv[]) {  
    if (argc != 3) {  
        std::cerr << "Usage: " << argv[0] << " <server_ip> <num_requests>" << std::endl;  
        return 1;  
    }  
    strncpy(SERVER_IP, argv[1], sizeof(SERVER_IP));

    int num_requests = std::stoi(argv[2]);  
    if (num_requests <= 0) {  
        std::cerr << "Invalid number of requests" << std::endl;  
        return 1;  
    }  
  
    std::vector<std::thread> threads;  
  
    // 假设我们想要限制并发线程数，这里为了简单起见，我们直接创建num_requests个线程  
    for (int i = 0; i < num_requests; ++i) {  
        threads.emplace_back(send_message, 1); // 每个线程只发送一个请求  
    }  
  
    // 等待所有线程完成  
    for (auto& t : threads) {  
        t.join();  
    }  
  
    return 0;  
}
