#include "io_multiplexing.h"
#include "db_manager.h"
#include "logger.h"
#include "thread_pool.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "json.hpp"
#include <execinfo.h>
#include <cxxabi.h>

using json = nlohmann::json;
using utils::ThreadPool;

namespace {
std::atomic<bool> running{true};
std::atomic<bool> reload_config{false};

// 获取堆栈信息的辅助函数
std::string getStackTrace() {
    const int max_frames = 32;
    void* array[max_frames];
    int nptrs = backtrace(array, max_frames);
    char** strings = backtrace_symbols(array, nptrs);
    
    if (strings == nullptr) {
        return "Failed to get backtrace symbols";
    }

    std::string result;
    for (int i = 0; i < nptrs; i++) {
        std::string symbol(strings[i]);
        size_t nameStart = symbol.find('(');
        size_t nameEnd = symbol.find('+', nameStart);
        
        if (nameStart != std::string::npos && nameEnd != std::string::npos) {
            std::string mangledName = symbol.substr(nameStart + 1, nameEnd - nameStart - 1);
            int status;
            char* demangled = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);
            
            if (status == 0 && demangled != nullptr) {
                result += std::to_string(i) + ": " + demangled + "\n";
                free(demangled);
            } else {
                result += std::to_string(i) + ": " + strings[i] + "\n";
            }
        } else {
            result += std::to_string(i) + ": " + strings[i] + "\n";
        }
    }
    
    free(strings);
    return result;
}

void crashHandler(int signum) {
    std::string signame;
    switch (signum) {
        case SIGSEGV: signame = "SIGSEGV"; break;
        case SIGABRT: signame = "SIGABRT"; break;
        case SIGFPE:  signame = "SIGFPE";  break;
        case SIGILL:  signame = "SIGILL";  break;
        case SIGBUS:  signame = "SIGBUS";  break;
        default:      signame = "Unknown signal ";
    }
    
    auto& logger = utils::Logger::getInstance();
    logger.error("Received fatal signal: " + signame + "\nStack trace:\n" + getStackTrace());
    
    // 恢复默认处理程序并重新发送信号
    signal(signum, SIG_DFL);
    raise(signum);
}

void signalHandler(int signum) {
    if (signum == SIGTERM || signum == SIGINT) {
        running = false;
    } else if (signum == SIGHUP) {
        reload_config = true;
    }
}

// 设置socket为非阻塞模式
bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}



// 服务器类
class Server {
public:
    Server(const std::string& config_path = "config/server_config.json")
        : port_(8080), threadPool_(4), dbManager_(), io_(nullptr) {
        logger_ = &utils::Logger::getInstance();
        loadConfig(config_path);
        logger_->info("Server initialized with configuration");
    }

    void loadConfig(const std::string& config_path) {
        try {
            std::ifstream config_file(config_path);
            if (!config_file.is_open()) {
                throw std::runtime_error("Unable to open config file: " + config_path);
            }

            json config = json::parse(config_file);
            port_ = config["server"]["port"];
            threadPool_.resize(config["server"]["num_threads"]);

            std::string log_file = config["logging"]["log_file"];
            logger_->setLogFile(log_file);

            // 更新数据库配置
            auto mysql_config = config["database"]["mysql"];
            dbManager_.updateMySQLConfig(
                mysql_config["host"],
                mysql_config["user"],
                mysql_config["password"],
                mysql_config["database"]);

            auto redis_config = config["database"]["redis"];
            dbManager_.updateRedisConfig(
                redis_config["host"],
                redis_config["port"]);

            logger_->info("Configuration loaded successfully");
        } catch (const std::exception& e) {
            logger_->error("Failed to load config: " + std::string(e.what()));
            throw;
        }
    }

    bool init() {
        // 创建服务器socket
        serverfd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverfd_ < 0) {
            logger_->error("Failed to create socket");
            return false;
        }

        // 设置socket选项
        int opt = 1;
        if (setsockopt(serverfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            logger_->error("Failed to set socket options");
            return false;
        }

        // 绑定地址
        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);

        if (bind(serverfd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            logger_->error("Failed to bind");
            return false;
        }

        // 监听连接
        if (listen(serverfd_, SOMAXCONN) < 0) {
            logger_->error("Failed to listen");
            return false;
        }

        // 设置为非阻塞模式
        if (!setNonBlocking(serverfd_)) {
            logger_->error("Failed to set non-blocking");
            return false;
        }

        // 初始化数据库连接
        if (!dbManager_.connectMySQL()) {
            logger_->error("Failed to connect to MySQL");
            return false;
        }
        if (!dbManager_.connectRedis()) {
            logger_->error("Failed to connect to Redis");
            return false;
        }

        // 创建IO多路复用对象
        io_ = std::make_unique<io::EpollIO>();
        io_->addEvent(serverfd_, io::EventType::READ);

        logger_->info("Server initialized on port " + std::to_string(port_));
        return true;
    }

    void run() {
        logger_->info("Server started on port " + std::to_string(port_));

        // 主事件循环
        while (running) {
            std::vector<io::Event> events;
            int nfds = io_->wait(events, 1000); // 1秒超时

            if (nfds < 0) {
                if (errno == EINTR) continue;
                logger_->error("IO wait error");
                break;
            }

            for (const auto& event : events) {
                if (event.fd == serverfd_) {
                    handleNewConnection();
                } else {
                    // 将客户端请求加入线程池
                    threadPool_.enqueue([this, fd = event.fd] {
                        handleClient(fd);
                    });
                }
            }
        }

        cleanup();
    }

private:
    void handleNewConnection() {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int clientfd = accept(serverfd_, (struct sockaddr*)&client_addr, &client_len);
        
        if (clientfd < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                logger_->error("Accept error");
            }
            return;
        }

        if (!setNonBlocking(clientfd)) {
            logger_->error("Failed to set client socket non-blocking");
            close(clientfd);
            return;
        }

        io_->addEvent(clientfd, io::EventType::READ);
    }

    void handleClient(int clientfd) {
        char buffer[1024];
        ssize_t n = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(clientfd);
            return;
        }
        buffer[n] = '\0';

        try {
            // 解析JSON请求
            json request = json::parse(buffer);
            std::string key = request["key"].get<std::string>();
            std::string value = request["value"].get<std::string>();

            // 先写入Redis缓存
            if (!dbManager_.setCache(key, value)) {
                throw std::runtime_error("Failed to set cache");
            }

            // 再写入MySQL
            std::string sql = "INSERT INTO data (key_name, value) VALUES ('" + 
                             key + "', '" + value + "')";
            if (!dbManager_.executeSQL(sql)) {
                throw std::runtime_error("Failed to execute SQL");
            }

            // 发送成功响应
            json response = {{
                {"status", "success"},
                {"message", "Data processed successfully"}
            }};
            std::string resp_str = response.dump();
            send(clientfd, resp_str.c_str(), resp_str.length(), 0);

        } catch (const std::exception& e) {
            // 发送错误响应
            json response = {{
                {"status", "error"},
                {"message", e.what()}
            }};
            std::string resp_str = response.dump();
            send(clientfd, resp_str.c_str(), resp_str.length(), 0);
        }

        close(clientfd);
    }

    void cleanup() {
        close(serverfd_);
    }

    int port_;
    int serverfd_;
    ThreadPool threadPool_;
    db::DBManager dbManager_;
    std::unique_ptr<io::EpollIO> io_;
    utils::Logger* logger_;
};

} // anonymous namespace

int main(int argc, char* argv[]) {
    std::string config_path = "config/server_config.json";
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            config_path = argv[++i];
        }
    }

    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGHUP, signalHandler);
    
    // 设置崩溃信号处理
    signal(SIGSEGV, crashHandler);
    signal(SIGABRT, crashHandler);
    signal(SIGFPE, crashHandler);
    signal(SIGILL, crashHandler);
    signal(SIGBUS, crashHandler);

    // 创建并启动服务器
    Server server(config_path);
    if (!server.init()) {
        return 1;
    }

    // 在独立线程中运行服务器
    std::thread server_thread([&server] {
        server.run();
    });
    server_thread.detach();

    // 主循环等待信号
    while (running) {
        if (reload_config) {
            try {
                server.loadConfig(config_path);
                reload_config = false;
            } catch (const std::exception& e) {
                std::cerr << "Failed to reload config: " << e.what() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}