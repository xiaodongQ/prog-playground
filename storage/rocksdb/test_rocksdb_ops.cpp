#include <iostream>
#include <string>
#include <cstdio>
#include <cassert>
#include "rocksdb/db.h"
#include "rocksdb/statistics.h"

void test() {
    rocksdb::DB* db;
    rocksdb::Options options;
    // 创建统计
    options.statistics = rocksdb::CreateDBStatistics();
    // 默认等级是kExceptDetailedTimers，不统计锁和压缩的耗时
    // options.statistics->set_stats_level(rocksdb::StatsLevel::kAll);

    options.create_if_missing = true;
    // open数据库，不存在则创建
    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

    std::string value;
    std::string key1 = "xdkey1";
    // 临时新增
    db->Put(rocksdb::WriteOptions(), key1, "test12345");
    // 读取key1
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key1, &value);
    if (s.ok()) {
        std::cout << "key:" << key1 << ", value:" << value << std::endl;
    } else if(s.code() == rocksdb::Status::kNotFound) {
        std::cout << "key:" << key1 << " not found" << std::endl;
    } else {
        std::cout << "key:" << key1 << " get error:" << s.code() << std::endl;
    }

    // 读取到key1，则将其value写入key2，并删除key1。可用rocksdb::WriteBatch来实现原子更新。
    std::string key2 = "xdkey2";
    if (s.ok()) {
        s = db->Put(rocksdb::WriteOptions(), key2, value);
        std::cout << "put key:" << key2 << " value:" << value << std::endl;
        // 而后删除key1
        if (s.ok()) {
            s = db->Delete(rocksdb::WriteOptions(), key1);
            std::cout << "delete key:" << key1 << std::endl;
        }
    }
    // 读取key2
    s = db->Get(rocksdb::ReadOptions(), key2, &value);
    if (s.ok()) {
        std::cout << "get key:" << key2 << ", value:" << value << std::endl;
    }


    // ========== 统计begin ============
    // 打印statistics统计信息
    // 直方图，支持很多种类型，由传入参数指定，此处为获取耗时
    std::cout << "histgram:\n" << options.statistics->getHistogramString(rocksdb::Histograms::DB_GET) << std::endl;
    // 统计字段
    std::cout << "statistics:\n" << options.statistics->ToString() << std::endl;
    // ========== 统计end ============

    // 关闭数据库
    status = db->Close();
    delete db;
}

int main(int argc, char *argv[])
{
    printf("====== begin... =====\n");
    test();
    printf("====== end ======\n");
    return 0;
}
