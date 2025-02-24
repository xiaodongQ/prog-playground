#include "thread_pool.h"

namespace utils {

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] {
                        return stop || !tasks.empty();
                    });
                    
                    if (stop && tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool::resize(size_t numThreads) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    // 等待所有任务完成
    while (!tasks.empty()) {
        condition.wait(lock);
    }
    // 停止当前所有线程
    stop = true;
    condition.notify_all();
    lock.unlock();
    
    // 等待所有线程结束
    for(std::thread &worker: workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
    
    // 重置线程池状态
    workers.clear();
    stop = false;
    
    // 创建新的线程
    for(size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while(true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    if(this->stop && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

} // namespace utils