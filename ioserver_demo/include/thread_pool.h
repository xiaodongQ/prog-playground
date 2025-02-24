#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>

namespace utils {

class ThreadPool {
public:
    // 构造函数，创建指定数量的工作线程
    explicit ThreadPool(size_t numThreads);

    // 析构函数，确保所有任务完成并停止所有线程
    ~ThreadPool();

    // 禁用拷贝构造和赋值操作
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 调整线程池大小
    void resize(size_t numThreads);

    // 提交任务到线程池
    // 返回一个future对象，可以用来获取任务的执行结果
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    // 工作线程向量
    std::vector<std::thread> workers;
    // 任务队列
    std::queue<std::function<void()>> tasks;
    
    // 同步相关成员
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};



// 任务提交函数实现
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}

} // namespace utils

#endif // THREAD_POOL_H