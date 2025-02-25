#include "db_manager.h"
#include <stdexcept>

namespace db {

DBManager::DBManager() : mysql_(nullptr), redis_(nullptr){
    logger_ = &utils::Logger::getInstance();
}

DBManager::~DBManager() {
    disconnect();
}

void DBManager::disconnect() {
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
        logger_->error("Failed to initialize MySQL connection");
        return false;
    }

    if (!mysql_real_connect(mysql_, mysql_host_.c_str(), mysql_user_.c_str(),
                          mysql_password_.c_str(), mysql_database_.c_str(),
                          mysql_port_, nullptr, 0)) {
        logger_->error("Failed to connect to MySQL: " + std::string(mysql_error(mysql_)));
        mysql_close(mysql_);
        mysql_ = nullptr;
        return false;
    }
    logger_->info("Successfully connected to MySQL database: " + mysql_database_);
    return true;
}

bool DBManager::executeSQL(const std::string& sql) {
    if (!mysql_) {
        logger_->error("Cannot execute SQL: MySQL connection not established");
        return false;
    }

    if (mysql_query(mysql_, sql.c_str()) != 0) {
        logger_->error("Failed to execute SQL: " + std::string(mysql_error(mysql_)) + "\nSQL: " + sql);
        return false;
    }
    logger_->debug("Successfully executed SQL: " + sql);
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
        std::string error_msg = redis_ ? std::string(redis_->errstr) : "Could not allocate redis context";
        logger_->error("Failed to connect to Redis: " + error_msg);
        if (redis_) {
            redisFree(redis_);
            redis_ = nullptr;
        }
        return false;
    }
    logger_->info("Successfully connected to Redis server at " + redis_host_ + ":" + std::to_string(redis_port_));
    return true;
}

bool DBManager::setCache(const std::string& key, const std::string& value) {
    if (!redis_) {
        logger_->error("Cannot set cache: Redis connection not established");
        return false;
    }

    redisReply* reply = (redisReply*)redisCommand(redis_, "SET %s %s",
                                                key.c_str(), value.c_str());
    if (!reply) {
        logger_->error("Failed to set cache for key: " + key);
        return false;
    }

    bool success = (reply->type != REDIS_REPLY_ERROR);
    if (!success) {
        logger_->error("Redis SET command failed for key: " + key);
    } else {
        logger_->debug("Successfully set cache for key: " + key);
    }
    
    freeReplyObject(reply);
    return success;
}

std::string DBManager::getCache(const std::string& key) {
    if (!redis_) {
        logger_->error("Cannot get cache: Redis connection not established");
        return "";
    }

    redisReply* reply = (redisReply*)redisCommand(redis_, "GET %s", key.c_str());
    if (!reply) {
        logger_->error("Failed to get cache for key: " + key);
        return "";
    }

    std::string value;
    if (reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
        logger_->debug("Successfully retrieved cache for key: " + key);
    } else {
        logger_->debug("Cache miss for key: " + key);
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