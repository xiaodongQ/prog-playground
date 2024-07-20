#include <cassert>
#include <iostream>

// db.h里包含了iterator.h
#include "leveldb/db.h"

// 针对用例：test_leveldb_write_batch
#include "leveldb/write_batch.h"

using namespace std;

void test_leveldb()
{
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(s.ok());

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
	leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(s.ok());

	// 读写操作
	std::string value = "hello-leveldb";
	string key1 = "xdkey1";
	string key2 = "xdkey2";

	// 设置key1
	// leveldb::WriteOptions()默认构造一个sync为false的选项结构体
	// 声明为：Status Put(const WriteOptions& options, const Slice& key, const Slice& value);
	// class Slice是一个简单的包含指针和数据大小的类结构，可以通过char*/string来构造初始化
	s = db->Put(leveldb::WriteOptions(), key1, value);
	assert(s.ok());
	cout << "set key:" << key1 << ", value:" << value << " ok" << endl;
	// 获取key1
	value="";
	// 声明为：Status Get(const ReadOptions& options, const Slice& key, std::string* value)
	s = db->Get(leveldb::ReadOptions(), key1, &value);
	assert(s.ok());
	cout << "get key:" << key1 << ", value:" << value << endl;

	// 设置key2的value为key1对应的value
	s = db->Put(leveldb::WriteOptions(), key2, value);
	assert(s.ok());
	cout << "set key:" << key2 << ", value:" << value << " ok" << endl;

	// 删除key1
	// 声明为：Status Delete(const WriteOptions& options, const Slice& key)
	s = db->Delete(leveldb::WriteOptions(), key1);
	assert(s.ok());
	cout << "del key:" << key1 << " ok"<< endl;

	// 尝试获取key1
	s = db->Get(leveldb::ReadOptions(), key1, &value);
	if (!s.ok()) {
		cout << "get key:" << key1 << " error! errmsg: " << s.ToString() << endl;
	}

	// 获取key2
	value = "";
	s = db->Get(leveldb::ReadOptions(), key2, &value);
	if (!s.ok()){
		cout << "get key:" << key2 << " error! errmsg: " << s.ToString() << endl;
	}else{
		cout << "get key:" << key2 << ", value:" << value << endl;
	}

	// 清理数据库
	delete db;
}

void test_leveldb_write_batch()
{
	// 场景：key1移动到key2
	// 过程为：获取key1的value -> 设置key2的value -> 删除key1。若删除key1之前db异常，则两个key有原来相同的value
	// 利用 `WriteBatch` 可达到原子更新的效果
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(s.ok());

	// 准备数据
	string key1 = "xdkey1";
	std::string value = "test-atomic-update";
	s = db->Put(leveldb::WriteOptions(), key1, value);
	assert(s.ok());
	cout << "set key:" << key1 << ", value:" << value << endl;

	string key2 = "xdkey2";
	value = "";
	s = db->Get(leveldb::ReadOptions(), key1, &value);
	assert(s.ok());

	// 使用 WriteBatch
	leveldb::WriteBatch batch;
	batch.Delete(key1);
	batch.Put(key2, value);
	s = db->Write(leveldb::WriteOptions(), &batch);
	assert(s.ok());
	cout << "move key:" << key1 << " to key:" << key2 << endl;

	s = db->Get(leveldb::ReadOptions(), key1, &value);
	cout << "get key: " << key1 << " result:" << s.ToString() << endl;
	s = db->Get(leveldb::ReadOptions(), key2, &value);
	cout << "get key: " << key2 << " result:" << s.ToString() << ", value:" << value << endl;

	// 清理数据库
	delete db;
}

void test_leveldb_iterator()
{
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	// 为防止之前记录影响观察，用个单独的数据库
	leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb_for_snapshot", &db);
	assert(s.ok());

	// 初始化数据
	s = db->Put(leveldb::WriteOptions(), "xdkey1", "itv1");
	assert(s.ok());
	s = db->Put(leveldb::WriteOptions(), "xdkey2", "itv2");
	assert(s.ok());
	s = db->Put(leveldb::WriteOptions(), "xdkey3", "itv3");
	assert(s.ok());
	s = db->Put(leveldb::WriteOptions(), "xdkey4", "itv4");
	assert(s.ok());

	// 创建 Iterator
	// 创建后是未初始化的，必须先调用某种Seek再使用它
	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	// 从头开始遍历所有记录，`SeekToFirst`
	cout << "scan first to last..." << endl;
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		// 注意Slice类需要调用下ToString()才转成std::string
		cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
		assert(it->status().ok());  // Check for any errors found during the scan
	}

	// 也可以从尾部开始向前遍历所有记录，`SeekToLast`
	cout << "scan last to first..." << endl;
	for (it->SeekToLast(); it->Valid(); it->Prev()) {
		cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
		assert(it->status().ok());  // Check for any errors found during the scan
	}

	// 也可以指定key的范围遍历，`Seek`并给定结束条件
	string start = "xdkey2";
	string end = "xdkey3";
	cout << "scan range [" << start << ", " << end << "]..." << endl;
	// 自行控制结束条件
	for (it->Seek(start); it->Valid() && it->key().ToString() <= end; it->Next()) {
		cout << it->key().ToString() << ": "  << it->value().ToString() << endl;
		assert(it->status().ok());  // Check for any errors found during the scan
	}

	delete it;

	// 清理数据库
	delete db;
}

void test_snapshot()
{
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status s = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(s.ok());

	// 初始化数据
	s = db->Put(leveldb::WriteOptions(), "xdkey1", "itv1");
	assert(s.ok());
	s = db->Put(leveldb::WriteOptions(), "xdkey2", "itv2");
	assert(s.ok());
	s = db->Put(leveldb::WriteOptions(), "xdkey3", "itv3");
	assert(s.ok());
	s = db->Put(leveldb::WriteOptions(), "xdkey4", "itv4");
	assert(s.ok());

	// options.snapshot
	leveldb::ReadOptions options_sp;
	// GetSnapshot生成快照
	// 声明为：const Snapshot* GetSnapshot()，返回一个当前db状态的处理句柄
	// 当不再使用时，必须通过 ReleaseSnapshot(result) 释放
	options_sp.snapshot = db->GetSnapshot();
	cout << "get snapshot" << endl;

	// 快照后做些增删改之类的操作，用作对比
	db->Put(leveldb::WriteOptions(), "xdkey5", "itv5");
	cout << "[add xdkey5:itv5]" << endl;
	assert(s.ok());
	db->Put(leveldb::WriteOptions(), "xdkey1", "itv1_modify");
	cout << "[modify xdkey1:itv1_modify]" << endl;
	assert(s.ok());
	db->Delete(leveldb::WriteOptions(), "xdkey2");
	cout << "[delete xdkey2]" << endl;
	assert(s.ok());
	
	// 此处使用的ReadOptions是设置了快照的，所以只有当时snapshot的记录状态
	cout << "scan snapshot..." << endl;
	leveldb::Iterator* it = db->NewIterator(options_sp);
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		cout << it->key().ToString() << ":" << it->value().ToString() << endl;
	}

	// 使用普通ReadOptions
	cout << "scan ordinarily..." << endl;
	leveldb::Iterator* it_od = db->NewIterator(leveldb::ReadOptions());
	for (it_od->SeekToFirst(); it_od->Valid(); it_od->Next()) {
		cout << it_od->key().ToString() << ":" << it_od->value().ToString() << endl;
	}
	delete it;

	// 释放快照
	db->ReleaseSnapshot(options_sp.snapshot);

    // 清理数据库
    delete db;
}

int main(int argc, char *argv[])
{
	// 打开关闭
	// test_leveldb();

	// 基本读写
	// test_leveldb_rw();

	// WriteBatch实现原子更新
	// test_leveldb_write_batch();

	// 迭代器:Iterator
	// test_leveldb_iterator();
	
	// 快照:Snapshot
	test_snapshot();
	



	return 0;
}
