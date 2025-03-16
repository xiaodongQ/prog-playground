#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>
#include <future>

using namespace std;

// 线程池类
class ThreadPool {
private:
    vector<thread> threads;
    deque<std::function<void()>> tasks;
    mutex task_mtx;
    condition_variable task_cond;
    std::atomic<bool> stop_;

public:
    // 构造函数，初始化线程池
    ThreadPool(int num) {
        stop_ = false;
        while (num-- > 0) {
            threads.emplace_back([this]() { thread_proc(); });
        }
    }

    // 析构函数，停止线程池并等待所有线程结束
    ~ThreadPool() {
        stop_ = true;
        task_cond.notify_all();
        for (auto &t : threads) {
            t.join();
        }
    }

    // 将任务加入线程池，并返回一个 std::future 对象用于获取任务结果
    template <class F, class... Args>
    auto enqueue_task(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        // std::packaged_task 包装一个可调用对象
        // 将一个可调用对象包装成一个异步任务，并提供一个 std::future 对象来获取任务的返回值。
        auto task = std::make_shared< std::packaged_task<return_type()> >(
            // std::bind 将任务函数和参数绑定在一起，
            // 然后将封装好的 std::packaged_task 包装成一个无参数的 std::function<void()> 并加入任务队列
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        // 返回std::packaged_task的 std::future对象，调用者可以通过该对象异步地获取任务的执行结果
        std::future<return_type> res = task->get_future();
        {
            unique_lock<mutex> lk(task_mtx);
            if (stop_)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace_back([task]() { (*task)(); });
        }
        task_cond.notify_one();
        return res;
    }

    // 停止线程池
    void stop() {
        stop_ = true;
        task_cond.notify_all();
    }

private:
    // 线程执行体
    void thread_proc() {
        while (!stop_) {
            std::function<void()> t;
            {
                unique_lock<mutex> lk(task_mtx);
                task_cond.wait(lk, [this]() { return stop_ || !tasks.empty(); });
                if (stop_ && tasks.empty()) {
                    return;
                }
                t = std::move(tasks.front());
                tasks.pop_front();
            }
            t();
        }
    }
};

// 任务执行函数
long long task_run(const std::vector<int> &data, int start, int end) {
    long long sum = 0;
    for (auto i = start; i < end; i++) {
        sum += data[i];
    }
    printf("start:%d, end:%d, chunk sum:%lld\n", start, end, sum);
    return sum;
}

int main(int argc, char *argv[]) {
    ThreadPool pool(4);
    std::vector<int> data(10000000, 2);
    size_t chunk = data.size() / 8;
    long long total_sum = 0;

    // 等待信号触发
    cout << "Press any key to start the tasks..." << endl;
    cin.get();

    // 存储所有任务的 future 对象
    std::vector<std::future<long long>> futures;

    // 信号触发后开始逻辑
    for (std::vector<int>::size_type i = 0; i < data.size(); i += chunk) {
        int end = std::min(i + chunk, data.size());
        // 将任务加入线程池并获取 future 对象
        futures.emplace_back(pool.enqueue_task(task_run, std::ref(data), i, end));
    }

    // 等待所有任务完成并累加结果
    for (auto& future : futures) {
        total_sum += future.get();
    }

    // 输出最终结果
    cout << "result: " << total_sum << endl;

    // 停止线程池
    pool.stop();

    return 0;
}
