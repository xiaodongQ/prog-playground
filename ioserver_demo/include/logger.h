#pragma once

#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/async.h"
#include <memory>
#include <string>
#include <vector>

namespace utils {

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogFile(const std::string& filename, bool console_output = true, const std::string& log_level = "debug") {
        try {
            std::vector<spdlog::sink_ptr> sinks;
            
            // Create console sink based on configuration
            if (console_output) {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                sinks.push_back(console_sink);
            }
            
            // Create file sink
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, 10*1024*1024, 5);
            sinks.push_back(file_sink);
            
            logger_ = std::make_shared<spdlog::logger>("main_logger", sinks.begin(), sinks.end());
            
            // Set log format
            // [2025-02-25 20:28:57.122][info|93252] Server initialized on port 8080
            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l|%t] %v");
            
            // Set log level according to configuration
            if (log_level == "trace") {
                logger_->set_level(spdlog::level::trace);
            } else if (log_level == "debug") {
                logger_->set_level(spdlog::level::debug);
            } else if (log_level == "info") {
                logger_->set_level(spdlog::level::info);
            } else if (log_level == "warn") {
                logger_->set_level(spdlog::level::warn);
            } else if (log_level == "error") {
                logger_->set_level(spdlog::level::err);
            } else if (log_level == "critical") {
                logger_->set_level(spdlog::level::critical);
            } else {
                logger_->set_level(spdlog::level::info); // Default to info level
            }
            
            spdlog::flush_every(std::chrono::seconds(3));
        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    void debug(const std::string& message) {
        if (logger_) logger_->debug(message);
    }

    void info(const std::string& message) {
        if (logger_) logger_->info(message);
    }

    void warn(const std::string& message) {
        if (logger_) logger_->warn(message);
    }

    void error(const std::string& message) {
        if (logger_) logger_->error(message);
    }

private:
    Logger() {
        spdlog::init_thread_pool(8192, 1);
        spdlog::flush_every(std::chrono::seconds(3));
    }
    ~Logger() {
        spdlog::shutdown();
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace utils