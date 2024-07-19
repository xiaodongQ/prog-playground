#include <cassert>
#include <iostream>
#include "leveldb/db.h"

using namespace std;

void test_leveldb()
{
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

	// 清理数据库
	delete db;
}

void test_leveldb_rw()
{
	leveldb::DB* db;

	// 初始化
	leveldb::Options options;
	options.create_if_missing = true;
	// 若已存在则DB::Open会报错退出
	// options.error_if_exists = true;
	leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(status.ok());

	// 读写操作
	std::string value="hello-leveldb";
	string key1="xdkey1";
	string key2="xdkey2";

	// 设置key1
	leveldb::Status s = db->Put(leveldb::WriteOptions(), key1, value);
	assert(s.ok());
	// 获取key1
	value="";
	s = db->Get(leveldb::ReadOptions(), key1, &value);
	cout << "key:" << key1 << ", value:" << value << endl;

	// 设置key2的value为key1对应的value
	// leveldb::WriteOptions()默认构造一个sync为false的选项结构体
	// 声明为：Status Put(const WriteOptions& options, const Slice& key, const Slice& value);
	// class Slice是一个简单的包含指针和数据大小的类结构，可以通过char*/string来构造初始化
	if (s.ok()) s = db->Put(leveldb::WriteOptions(), key2, value);

	// 删除key1
	if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);

	// 尝试获取key1
	s = db->Get(leveldb::ReadOptions(), key1, &value);
	if (!s.ok()) {
		cout << "get key:" << key1 << " error!" << endl;
	}

	// 获取key2
	s = db->Get(leveldb::ReadOptions(), key1, &value);
	cout << "key2:" << key1 << ", value:" << value << endl;

	// 清理数据库
	delete db;

}

int main(int argc, char *argv[])
{
	// test_leveldb();
	test_leveldb_rw();
	return 0;
}
