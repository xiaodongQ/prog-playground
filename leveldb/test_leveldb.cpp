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
}

int main(int argc, char *argv[])
{
	test_leveldb();
	return 0;
}
