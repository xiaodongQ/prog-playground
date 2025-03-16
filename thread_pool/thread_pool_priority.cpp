#include <iostream>
#include <vector>
// priority_queue也在该头文件
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>

using namespace std;

// 任务支持优先级处理
struct Task {
    std::function<void()> task;
    int priority;
    Task():priority(0) {}
    Task(int pri, std::function<void()> &&f) {
        priority = pri;
        task = std::move(f);
    }
    // const函数
    bool operator<(const Task &other) const {
        return priority < other.priority;
    }
};

// TODO 定义和实现暂放在一起，后续分离
class ThreadPool {
private:
    vector<thread> threads;
    priority_queue<Task> tasks;
    mutex task_mtx;
    condition_variable task_cond;
    std::atomic<bool> stop_;

public:
    ThreadPool(int num) {
        stop_ = false;
        while (num-- > 0) {
            threads.emplace_back([this]() { thread_proc(); });
        }
    }

    ~ThreadPool() {
        stop_ = true;
        task_cond.notify_all();
        for(auto &t : threads) {
            t.join();
        }
    }

    // 任务加入线程池
    void enqueue_task(int priority, std::function<void()> &&task) {
        {
            unique_lock<mutex> lk(task_mtx);
            // 使用移动语义以调用移动构造
            // {xxx} 列表初始化（List Initialization），此处会自动调用合适的构造函数
            tasks.push({priority, std::move(task)});
        }
        // 条件变量通知，既可以放在锁内，也可以放在锁外，各有优劣
        // 持锁内通知：确保其他唤醒线程是最新的共享状态；性能方面，释放锁其他线程才能被唤醒
        // 锁外通知：其他线程被唤醒后能更快地获取锁；需要确保释放锁后共享状态不会被意外修改
        // 一般建议将条件变量的通知操作放在锁外，以提高并发性能。
        task_cond.notify_one();
    }

    void stop() {
        stop_ = true;
        task_cond.notify_all();
    }

private:
    // 线程池执行体
    void thread_proc() {
        while(!stop_) {
            // 从任务队列获取任务
            Task t;
            {
                unique_lock<mutex> lk(task_mtx);
                task_cond.wait(lk, [this]() { return stop_ || !tasks.empty(); });
                // 外部停止线程池
                if(stop_ && tasks.empty()) {
                    return;
                }
                t = std::move(tasks.top());
                printf("tasks size:%d, priority:%d\n", tasks.size(), t.priority);
                tasks.pop();
            }
            // 执行任务
            t.task();
        }
    }
};

struct Result {
    long long sum;
    std::mutex mtx;
    int task_count;
    // 用于和task_count比较，所有都完成则进行通知
    int task_done_count;
    std::condition_variable cond;
    Result():sum(0), task_count(0), task_done_count(0) {}
};
void task_run(const std::vector<int> &data, int start, int end, Result &result) {
    long long sum = 0;
    for(auto i = start; i < end; i++) {
        sum += data[i];
    }
    
    lock_guard<mutex> lk(result.mtx);
    result.sum += sum;
    result.task_done_count++;
    // 等待，验证优先级是否生效
    std::this_thread::sleep_for(std::chrono::seconds(1));

    printf("start:%d, end:%d, chunk sum:%lld, total:%lld, done count:%d, task:%d\n", 
            start, end, sum, result.sum, result.task_done_count, result.task_count);
    if(result.task_done_count == result.task_count) {
        result.cond.notify_one();
    }
}

int main(int argc, char *argv[])
{
    ThreadPool pool(1);
    std::vector<int> data(10000000, 2);
    size_t chunk = data.size() / 8;
    Result result;
    // 记录要执行的任务数
    result.task_count = data.size()/chunk + ((data.size() % chunk == 0) ? 0 : 1);
    for(auto i = 0; i < data.size(); i += chunk) {
        // 循环不变量[start, end)
        int end = std::min(i + chunk, data.size());
        // 引用捕获result，其他按值捕获。且lambda函数无参数，和std::function<void()>一致
        // 设置部分任务的优先级
        pool.enqueue_task(i%3, [=, &result]() { task_run(data, i, end, result); });
    }

    {
        // 等待执行完成，通过信号量通知
        unique_lock<mutex> lock(result.mtx);
        result.cond.wait(lock);
        cout << "result: " << result.sum << endl;
    }
    pool.stop();
}
