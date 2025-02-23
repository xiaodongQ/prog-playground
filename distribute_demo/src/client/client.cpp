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
          success_count_(0), error_count_(0) {}

    TestResult run() {
        std::vector<std::thread> threads;
        std::vector<double> latencies;
        std::mutex latencies_mutex;

        auto start_time = high_resolution_clock::now();

        // 启动测试线程
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

        // 等待所有线程完成
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
        if (sockfd < 0) return false;

        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sockfd);
            return false;
        }

        // 准备测试数据
        json request = {
            {"key", "test_key_" + std::to_string(req_id)},
            {"value", "test_value_" + std::to_string(req_id)}
        };
        std::string req_str = request.dump();

        // 发送请求
        if (send(sockfd, req_str.c_str(), req_str.length(), 0) < 0) {
            close(sockfd);
            return false;
        }

        // 接收响应
        char buffer[1024];
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        close(sockfd);

        if (n <= 0) return false;

        buffer[n] = '\0';
        try {
            json response = json::parse(buffer);
            return response["status"] == "success";
        } catch (...) {
            return false;
        }
    }

    std::string host_;
    int port_;
    int num_threads_;
    int requests_per_thread_;
    std::atomic<size_t> success_count_;
    std::atomic<size_t> error_count_;
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

    std::string result_info = "\nBenchmark results:\n"
                             "Average latency: " + std::to_string(result.avg_latency_ms) + " ms\n"
                             "Throughput: " + std::to_string(result.throughput) + " requests/sec\n"
                             "Success requests: " + std::to_string(result.success_count) + "\n"
                             "Failed requests: " + std::to_string(result.error_count);
    logger.info(result_info);

    return 0;
}