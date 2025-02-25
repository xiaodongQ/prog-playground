#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "json.hpp"
#include "logger.h"

using json = nlohmann::json;
using namespace std::chrono;

struct TestResult {
    double avg_latency_ms;
    double throughput;
    size_t success_count;
    size_t error_count;
};

class BenchmarkClient {
public:
    BenchmarkClient(const std::string& host, int port,
                    int num_threads, int requests_per_thread)
        : host_(host), port_(port),
          num_threads_(num_threads),
          requests_per_thread_(requests_per_thread),
          success_count_(0), error_count_(0) {
        logger_ = &utils::Logger::getInstance();
    }

    TestResult run() {
        std::vector<std::thread> threads;
        std::vector<double> latencies;
        std::mutex latencies_mutex;

        auto start_time = high_resolution_clock::now();

        // Start test threads
        for (int i = 0; i < num_threads_; ++i) {
            threads.emplace_back([this, &latencies, &latencies_mutex]() {
                for (int j = 0; j < requests_per_thread_; ++j) {
                    auto req_start = high_resolution_clock::now();
                    bool success = sendRequest(j);
                    auto req_end = high_resolution_clock::now();

                    double latency = duration_cast<microseconds>(req_end - req_start).count() / 1000.0;

                    if (success) {
                        success_count_++;
                        std::lock_guard<std::mutex> lock(latencies_mutex);
                        latencies.push_back(latency);
                    } else {
                        error_count_++;
                    }
                }
            });
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        auto end_time = high_resolution_clock::now();
        double total_time = duration_cast<milliseconds>(end_time - start_time).count() / 1000.0;

        // 计算结果
        TestResult result;
        result.success_count = success_count_;
        result.error_count = error_count_;

        if (!latencies.empty()) {
            double total_latency = 0;
            for (double latency : latencies) {
                total_latency += latency;
            }
            result.avg_latency_ms = total_latency / latencies.size();
        } else {
            result.avg_latency_ms = 0;
        }

        result.throughput = success_count_ / total_time;
        return result;
    }

private:
    bool sendRequest(int req_id) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            logger_->error("Failed to create socket");
            return false;
        }

        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);

        // 添加连接重试机制
        int retry_count = 3;
        bool connected = false;
        while (retry_count-- > 0 && !connected) {
            if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
                connected = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!connected) {
            logger_->error("Failed to connect to server after retries, host=" + host_ + ", port=" + std::to_string(port_));
            close(sockfd);
            return false;
        }
        logger_->debug("Connected to server " + host_ + ":" + std::to_string(port_));
    
        // 准备批量测试数据
        json request;
        // const int batch_size = 5;  // 每次发送5个键值对
        const int batch_size = 1;
        json batch_data = json::array();
        for (int i = 0; i < batch_size; ++i) {
            batch_data.push_back({
                {"key", "test_key_" + std::to_string(req_id) + "_" + std::to_string(i)},
                {"value", "test_value_" + std::to_string(req_id) + "_" + std::to_string(i)}
            });
        }
        std::string req_str = batch_data.dump();
        logger_->debug("Sending request " + std::to_string(req_id) + ", size=" + std::to_string(req_str.length()) + " bytes");
        logger_->debug("request: [" + req_str + "]");
    
        // 发送请求，确保完整发送
        size_t total_sent = 0;
        while (total_sent < req_str.length()) {
            ssize_t sent = send(sockfd, req_str.c_str() + total_sent,
                              req_str.length() - total_sent, MSG_NOSIGNAL);
            if (sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;  // 等待可以继续发送
                }
                logger_->error("Error sending request: " + std::string(strerror(errno)) + ", errno=" + std::to_string(errno));
                close(sockfd);
                return false;
            }
            total_sent += sent;
            logger_->debug("Sent " + std::to_string(sent) + " bytes, total sent: " + std::to_string(total_sent));
        }

        // 接收响应，使用更大的缓冲区
        constexpr size_t BUFFER_SIZE = 8192;
        char buffer[BUFFER_SIZE];
        std::string response_data;

        while (true) {
            ssize_t n = recv(sockfd, buffer, BUFFER_SIZE, 0);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;  // 数据读取完毕
                }
                logger_->error("Error reading response: " + std::string(strerror(errno)));
                close(sockfd);
                return false;
            } else if (n == 0) {
                break;  // 连接关闭
            }
            response_data.append(buffer, n);
        }

        close(sockfd);

        // 添加调试日志，输出原始响应数据
        logger_->debug("Received raw response data: " + response_data);

        try {
            json response = json::parse(response_data);
            if (response["status"] == "success") {
                logger_->debug("Request " + std::to_string(req_id) + " succeeded");
                return true;
            } else {
                logger_->error("Request " + std::to_string(req_id) + " failed: " + 
                              response["message"].get<std::string>());
                return false;
            }
        } catch (const std::exception& e) {
            logger_->error("Failed to parse response: " + std::string(e.what()));
            return false;
        }
    }

    std::string host_;
    int port_;
    int num_threads_;
    int requests_per_thread_;
    std::atomic<size_t> success_count_;
    std::atomic<size_t> error_count_;
    utils::Logger* logger_;
};

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <num_threads> <requests_per_thread>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    int num_threads = std::stoi(argv[3]);
    int requests_per_thread = std::stoi(argv[4]);

    auto& logger = utils::Logger::getInstance();
    logger.setLogFile("client.log");
    
    std::string config_info = "Starting benchmark with:\n"
                             "Host: " + host + "\n"
                             "Port: " + std::to_string(port) + "\n"
                             "Threads: " + std::to_string(num_threads) + "\n"
                             "Requests per thread: " + std::to_string(requests_per_thread);
    logger.info(config_info);

    BenchmarkClient client(host, port, num_threads, requests_per_thread);
    TestResult result = client.run();

    std::string result_info = "\n=========================";
    result_info += "\nBenchmark results:\n"
                             "Average latency: " + std::to_string(result.avg_latency_ms) + " ms\n"
                             "Throughput: " + std::to_string(result.throughput) + " requests/sec\n"
                             "Success requests: " + std::to_string(result.success_count) + "\n"
                             "Failed requests: " + std::to_string(result.error_count);
    result_info += "\n=========================";
    logger.info(result_info);

    return 0;
}