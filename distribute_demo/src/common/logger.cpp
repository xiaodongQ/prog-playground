#include "logger.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace utils {

Logger::Logger() : min_level_(LogLevel::INFO), running_(true) {
    write_thread_ = std::make_unique<std::thread>(&Logger::asyncWrite, this);
}

Logger::~Logger() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        running_ = false;
        cv_.notify_one();
    }
    if (write_thread_ && write_thread_->joinable()) {
        write_thread_->join();
    }
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

void Logger::setLogFile(const std::string& filename, size_t max_file_size) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_.close();
    }
    log_filename_ = filename;
    max_file_size_ = max_file_size;
    log_file_.open(filename, std::ios::app);
}

void Logger::setLogLevel(LogLevel level) {
    std::unique_lock<std::mutex> lock(mutex_);
    min_level_ = level;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    return ss.str();
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < min_level_) return;
    
    std::string log_entry = getCurrentTimestamp() + " [" + getLevelString(level) + "] " + message + "\n";
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        message_queue_.push(log_entry);
    }
    cv_.notify_one();
}

void Logger::asyncWrite() {
    while (true) {
        std::string message;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !message_queue_.empty() || !running_; });
            
            if (!running_ && message_queue_.empty()) {
                break;
            }
            
            if (!message_queue_.empty()) {
                message = std::move(message_queue_.front());
                message_queue_.pop();
            }
        }
        
        if (!message.empty()) {
            if (log_file_.is_open()) {
                log_file_ << message;
                log_file_.flush();
                rotateLogFile();
            }
            std::cout << message;
        }
    }
}

void Logger::rotateLogFile() {
    if (!log_file_.is_open() || log_filename_.empty()) return;
    
    log_file_.seekp(0, std::ios::end);
    if (log_file_.tellp() >= static_cast<std::streampos>(max_file_size_)) {
        log_file_.close();
        std::string backup_filename = log_filename_ + "." + 
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        std::rename(log_filename_.c_str(), backup_filename.c_str());
        log_file_.open(log_filename_, std::ios::app);
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

} // namespace utils