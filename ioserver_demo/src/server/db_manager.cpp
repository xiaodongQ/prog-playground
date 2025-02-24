#include "db_manager.h"
#include <stdexcept>
#include "logger.h"

namespace db {

DBManager::DBManager() : mysql_(nullptr), redis_(nullptr) {}

DBManager::~DBManager() {
    if (mysql_) {
        mysql_close(mysql_);
        mysql_ = nullptr;
    }
    if (redis_) {
        redisFree(redis_);
        redis_ = nullptr;
    }
}

bool DBManager::connectMySQL() {
    mysql_ = mysql_init(nullptr);
    if (!mysql_) {
        return false;
    }

    if (!mysql_real_connect(mysql_, mysql_host_.c_str(), mysql_user_.c_str(),
                          mysql_password_.c_str(), mysql_database_.c_str(),
                          mysql_port_, nullptr, 0)) {
        mysql_close(mysql_);
        mysql_ = nullptr;
        return false;
    }
    return true;
}

bool DBManager::executeSQL(const std::string& sql) {
    if (!mysql_) {
        return false;
    }

    if (mysql_query(mysql_, sql.c_str()) != 0) {
        return false;
    }
    return true;
}

bool DBManager::query(const std::string& sql) {
    if (!mysql_) {
        return false;
    }

    if (mysql_query(mysql_, sql.c_str()) != 0) {
        return false;
    }

    MYSQL_RES* result = mysql_store_result(mysql_);
    if (!result) {
        return false;
    }

    mysql_free_result(result);
    return true;
}

bool DBManager::connectRedis() {
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    redis_ = redisConnectWithTimeout(redis_host_.c_str(), redis_port_, timeout);
    
    if (!redis_ || redis_->err) {
        if (redis_) {
            redisFree(redis_);
            redis_ = nullptr;
        }
        return false;
    }
    return true;
}

bool DBManager::setCache(const std::string& key, const std::string& value) {
    if (!redis_) {
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(redis_, "SET %s %s",
                                                key.c_str(), value.c_str());
    if (!reply) {
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success) {
    }
    
    freeReplyObject(reply);
    return success;
}

std::string DBManager::getCache(const std::string& key) {
    if (!redis_) {
        return "";
    }

    redisReply* reply = (redisReply*)redisCommand(redis_, "GET %s", key.c_str());
    if (!reply) {
        return "";
    }

    std::string value;
    if (reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
    }

    freeReplyObject(reply);
    return value;
}

void DBManager::updateMySQLConfig(const std::string& host, const std::string& user,
                                const std::string& password, const std::string& database,
                                unsigned int port) {
    mysql_host_ = host;
    mysql_user_ = user;
    mysql_password_ = password;
    mysql_database_ = database;
    mysql_port_ = port;
}

void DBManager::updateRedisConfig(const std::string& host, int port) {
    redis_host_ = host;
    redis_port_ = port;
}

} // namespace db