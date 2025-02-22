#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include <memory>
#include <mysql/mysql.h>
#include <hiredis/hiredis.h>

namespace db {

class DBManager {
public:
    DBManager();
    ~DBManager();

    // MySQL操作
    bool connectMySQL(const std::string& host, const std::string& user,
                     const std::string& password, const std::string& database,
                     unsigned int port = 3306);
    bool executeSQL(const std::string& sql);
    bool query(const std::string& sql);

    // Redis操作
    bool connectRedis(const std::string& host = "127.0.0.1", int port = 6379);
    bool setCache(const std::string& key, const std::string& value);
    std::string getCache(const std::string& key);

    // 配置更新
    void updateMySQLConfig(const std::string& host, const std::string& user,
                          const std::string& password, const std::string& database,
                          unsigned int port = 3306);
    void updateRedisConfig(const std::string& host, int port = 6379);

private:
    MYSQL* mysql_;
    redisContext* redis_;
    
    // MySQL配置
    std::string mysql_host_;
    std::string mysql_user_;
    std::string mysql_password_;
    std::string mysql_database_;
    unsigned int mysql_port_;
    
    // Redis配置
    std::string redis_host_;
    int redis_port_;
};

} // namespace db

#endif // DB_MANAGER_H