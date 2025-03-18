#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>

std::mutex mtx;
int shared_variable = 0;

void worker(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        // 模拟一些工作
        ++shared_variable;
    }
}

int main() {
    const int num_threads = 4;
    const int iterations = 1000000;

    std::vector<std::thread> threads;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 创建并启动线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, iterations);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Total time: " << duration << " ms" << std::endl;
    std::cout << "Final shared variable value: " << shared_variable << std::endl;

    return 0;
}    
