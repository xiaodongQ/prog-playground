#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <sstream>

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
    std::string getCurrentTimestamp();
    std::string getLevelString(LogLevel level);
    void rotateLogFile();

    std::ofstream log_file_;
    std::string log_filename_;
    size_t max_file_size_;
    LogLevel min_level_;

    std::queue<std::string> message_queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_;
    std::unique_ptr<std::thread> write_thread_;
};

} // namespace utils

#endif // LOGGER_H