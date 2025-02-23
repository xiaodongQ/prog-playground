#ifndef IO_MULTIPLEXING_H
#define IO_MULTIPLEXING_H

#include <vector>
#include <functional>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/poll.h>

namespace io {

// IO事件类型
enum class EventType {
    READ = 1,
    WRITE = 2,
    ERROR = 4
};

// IO事件结构
struct Event {
    int fd;             // 文件描述符
    EventType type;     // 事件类型
    void* data;         // 用户数据
};

// IO多路复用接口抽象类
class IOMultiplexing {
public:
    virtual ~IOMultiplexing() = default;
    
    // 添加监听事件
    virtual bool addEvent(int fd, EventType type) = 0;
    
    // 移除监听事件
    virtual bool removeEvent(int fd, EventType type) = 0;
    
    // 修改监听事件
    virtual bool modifyEvent(int fd, EventType type) = 0;
    
    // 等待事件发生
    virtual int wait(std::vector<Event>& events, int timeout = -1) = 0;
};

// Select实现
class SelectIO : public IOMultiplexing {
private:
    fd_set readfds_, writefds_, errorfds_;
    int maxfd_;
    
public:
    SelectIO();
    ~SelectIO() override;
    
    bool addEvent(int fd, EventType type) override;
    bool removeEvent(int fd, EventType type) override;
    bool modifyEvent(int fd, EventType type) override;
    int wait(std::vector<Event>& events, int timeout = -1) override;
};

// Poll实现
class PollIO : public IOMultiplexing {
private:
    std::vector<pollfd> pollfds_;
    
public:
    PollIO();
    ~PollIO() override;
    
    bool addEvent(int fd, EventType type) override;
    bool removeEvent(int fd, EventType type) override;
    bool modifyEvent(int fd, EventType type) override;
    int wait(std::vector<Event>& events, int timeout = -1) override;
};

// Epoll实现
class EpollIO : public IOMultiplexing {
private:
    int epollfd_;
    std::vector<epoll_event> events_;
    
public:
    EpollIO(int max_events = 1024);
    ~EpollIO() override;
    
    bool addEvent(int fd, EventType type) override;
    bool removeEvent(int fd, EventType type) override;
    bool modifyEvent(int fd, EventType type) override;
    int wait(std::vector<Event>& events, int timeout = -1) override;
};

} // namespace io

#endif // IO_MULTIPLEXING_H