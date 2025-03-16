#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>

using namespace std;

// 线程池类
class ThreadPool {
private:
    vector<thread> threads;
    deque<std::function<void()>> tasks;
    mutex task_mtx;
    condition_variable task_cond;
    std::atomic<bool> stop_;
    std::atomic<bool> started_;

public:
    // 构造函数，初始化线程池
    ThreadPool(int num) {
        stop_ = false;
        started_ = false;
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

    // 将任务加入线程池
    void enqueue_task(std::function<void()> &&task) {
        {
            unique_lock<mutex> lk(task_mtx);
            tasks.emplace_back(std::move(task));
        }
        task_cond.notify_one();
    }

    // 启动线程池
    void start() {
        started_ = true;
        task_cond.notify_all();
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
                task_cond.wait(lk, [this]() { return stop_ || (started_ &&!tasks.empty()); });
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

// 结果结构体，用于存储计算结果和同步信息
struct Result {
    long long sum;
    std::mutex mtx;
    int task_count;
    int task_done_count;
    std::condition_variable cond;
    Result() : sum(0), task_count(0), task_done_count(0) {}
};

// 任务执行函数
void task_run(const std::vector<int> &data, int start, int end, Result &result) {
    long long sum = 0;
    for (auto i = start; i < end; i++) {
        sum += data[i];
    }

    lock_guard<mutex> lk(result.mtx);
    result.sum += sum;
    result.task_done_count++;
    printf("start:%d, end:%d, chunk sum:%lld, total:%lld, done count:%d, task:%d\n",
           start, end, sum, result.sum, result.task_done_count, result.task_count);
    if (result.task_done_count == result.task_count) {
        result.cond.notify_one();
    }
}

int main(int argc, char *argv[]) {
    ThreadPool pool(4);
    std::vector<int> data(10000000, 2);
    size_t chunk = data.size() / 8;
    Result result;

    // 等待信号触发
    cout << "Press any key to start the tasks..." << endl;
    cin.get();

    // 信号触发后开始逻辑
    result.task_count = data.size() / chunk + ((data.size() % chunk == 0) ? 0 : 1);
    for (std::vector<int>::size_type i = 0; i < data.size(); i += chunk) {
        int end = std::min(i + chunk, data.size());
        pool.enqueue_task([=, &result]() { task_run(data, i, end, result); });
    }

    // 启动线程池
    pool.start();

    // 等待所有任务完成
    {
        unique_lock<mutex> lock(result.mtx);
        result.cond.wait(lock);
        cout << "result: " << result.sum << endl;
    }

    // 停止线程池
    pool.stop();

    return 0;
}
