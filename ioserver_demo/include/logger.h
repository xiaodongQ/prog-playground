#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>
#include <chrono>

namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename, size_t max_file_size = 10 * 1024 * 1024);
    void setLogLevel(LogLevel level);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    ~Logger();

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& message);
    void asyncWrite();
    void rotateLogFile();
    std::string getCurrentTimestamp();
    std::string getLevelString(LogLevel level);

    std::ofstream log_file_;
    std::string log_filename_;
    size_t max_file_size_;
    LogLevel min_level_;
    std::mutex mutex_;
    
    // 异步写入相关成员
    std::queue<std::string> message_queue_;
    std::condition_variable cv_;
    std::unique_ptr<std::thread> write_thread_;
    bool running_;
};

} // namespace utils

#endif // LOGGER_H