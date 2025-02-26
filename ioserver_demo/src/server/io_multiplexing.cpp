#include "io_multiplexing.h"
#include <stdexcept>
#include <cstring>
#include <unistd.h>

namespace io {

// SelectIO实现
SelectIO::SelectIO() : maxfd_(-1) {
    FD_ZERO(&readfds_);
    FD_ZERO(&writefds_);
    FD_ZERO(&errorfds_);
}

SelectIO::~SelectIO() = default;

bool SelectIO::addEvent(int fd, EventType type) {
    if (fd < 0) return false;
    
    if (static_cast<int>(type) & static_cast<int>(EventType::READ))
        FD_SET(fd, &readfds_);
    if (static_cast<int>(type) & static_cast<int>(EventType::WRITE))
        FD_SET(fd, &writefds_);
    if (static_cast<int>(type) & static_cast<int>(EventType::ERROR))
        FD_SET(fd, &errorfds_);
    
    maxfd_ = std::max(maxfd_, fd);
    return true;
}

bool SelectIO::removeEvent(int fd, EventType type) {
    if (fd < 0) return false;
    
    if (static_cast<int>(type) & static_cast<int>(EventType::READ))
        FD_CLR(fd, &readfds_);
    if (static_cast<int>(type) & static_cast<int>(EventType::WRITE))
        FD_CLR(fd, &writefds_);
    if (static_cast<int>(type) & static_cast<int>(EventType::ERROR))
        FD_CLR(fd, &errorfds_);
    
    return true;
}

bool SelectIO::modifyEvent(int fd, EventType type) {
    return removeEvent(fd, type) && addEvent(fd, type);
}

int SelectIO::wait(std::vector<Event>& events, int timeout) {
    fd_set rfds = readfds_;
    fd_set wfds = writefds_;
    fd_set efds = errorfds_;
    
    struct timeval tv, *ptv = nullptr;
    if (timeout >= 0) {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        ptv = &tv;
    }
    
    int ret = select(maxfd_ + 1, &rfds, &wfds, &efds, ptv);
    if (ret < 0) return -1;
    
    events.clear();
    for (int fd = 0; fd <= maxfd_; ++fd) {
        Event event{fd, EventType::READ, nullptr};
        if (FD_ISSET(fd, &rfds)) {
            event.type = EventType::READ;
            events.push_back(event);
        }
        if (FD_ISSET(fd, &wfds)) {
            event.type = EventType::WRITE;
            events.push_back(event);
        }
        if (FD_ISSET(fd, &efds)) {
            event.type = EventType::ERROR;
            events.push_back(event);
        }
    }
    
    return ret;
}

// PollIO实现
PollIO::PollIO() = default;

PollIO::~PollIO() = default;

bool PollIO::addEvent(int fd, EventType type) {
    if (fd < 0) return false;
    
    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = 0;
    
    if (static_cast<int>(type) & static_cast<int>(EventType::READ))
        pfd.events |= POLLIN;
    if (static_cast<int>(type) & static_cast<int>(EventType::WRITE))
        pfd.events |= POLLOUT;
    if (static_cast<int>(type) & static_cast<int>(EventType::ERROR))
        pfd.events |= POLLERR;
    
    pollfds_.push_back(pfd);
    return true;
}

bool PollIO::removeEvent(int fd, [[maybe_unused]] EventType type) {
    if (fd < 0) return false;
    
    for (auto it = pollfds_.begin(); it != pollfds_.end();) {
        if (it->fd == fd) {
            it = pollfds_.erase(it);
        } else {
            ++it;
        }
    }
    return true;
}

bool PollIO::modifyEvent(int fd, EventType type) {
    return removeEvent(fd, type) && addEvent(fd, type);
}

int PollIO::wait(std::vector<Event>& events, int timeout) {
    int ret = poll(pollfds_.data(), pollfds_.size(), timeout);
    if (ret < 0) return -1;
    
    events.clear();
    for (const auto& pfd : pollfds_) {
        if (pfd.revents & (POLLIN | POLLOUT | POLLERR)) {
            Event event{pfd.fd, EventType::READ, nullptr};
            if (pfd.revents & POLLIN)
                event.type = EventType::READ;
            else if (pfd.revents & POLLOUT)
                event.type = EventType::WRITE;
            else if (pfd.revents & POLLERR)
                event.type = EventType::ERROR;
            events.push_back(event);
        }
    }
    
    return ret;
}

// EpollIO实现
EpollIO::EpollIO(int max_events) : events_(max_events) {
    // 构造时创建epoll实例；且在初始化列表中初始化events_的vector大小
    epollfd_ = epoll_create1(0);
    if (epollfd_ < 0) {
        throw std::runtime_error("epoll_create1 failed");
    }
}

EpollIO::~EpollIO() {
    if (epollfd_ >= 0) {
        close(epollfd_);
    }
}

bool EpollIO::addEvent(int fd, EventType type) {
    if (fd < 0) return false;
    
    epoll_event ev{};
    ev.data.fd = fd;
    
    if (static_cast<int>(type) & static_cast<int>(EventType::READ))
        ev.events |= EPOLLIN;
    if (static_cast<int>(type) & static_cast<int>(EventType::WRITE))
        ev.events |= EPOLLOUT;
    if (static_cast<int>(type) & static_cast<int>(EventType::ERROR))
        ev.events |= EPOLLERR;
    
    return epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool EpollIO::removeEvent(int fd, [[maybe_unused]] EventType type) {
    if (fd < 0) return false;
    return epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

bool EpollIO::modifyEvent(int fd, EventType type) {
    if (fd < 0) return false;
    
    epoll_event ev{};
    ev.data.fd = fd;
    
    if (static_cast<int>(type) & static_cast<int>(EventType::READ))
        ev.events |= EPOLLIN;
    if (static_cast<int>(type) & static_cast<int>(EventType::WRITE))
        ev.events |= EPOLLOUT;
    if (static_cast<int>(type) & static_cast<int>(EventType::ERROR))
        ev.events |= EPOLLERR;
    
    return epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

int EpollIO::wait(std::vector<Event>& events, int timeout) {
    int ret = epoll_wait(epollfd_, events_.data(), events_.size(), timeout);
    if (ret < 0) return -1;
    
    events.clear();
    for (int i = 0; i < ret; ++i) {
        Event event{events_[i].data.fd, EventType::READ, nullptr};
        if (events_[i].events & EPOLLIN)
            event.type = EventType::READ;
        else if (events_[i].events & EPOLLOUT)
            event.type = EventType::WRITE;
        else if (events_[i].events & EPOLLERR)
            event.type = EventType::ERROR;
        events.push_back(event);
    }
    
    return ret;
}

} // namespace io