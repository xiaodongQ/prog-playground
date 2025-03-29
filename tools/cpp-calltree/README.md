# 工具说明

作者对工具的介绍：[C++阅码神器cpptree.pl和calltree.pl的使用](https://zhuanlan.zhihu.com/p/339910341)

* 使用`ag`替换`awk`，性能更高，其中使用pthread多线程处理、使用mmap而不是直接打开文件到缓存中
    * `yum install the_silver_searcher` 可安装`ag`
    * 使用作者修改过的ag：[the_silver_searcher](https://github.com/satanson/the_silver_searcher.git)，优化有些函数之间没有空格时的解析
* 使用`perl`处理正则

脚本所在仓库：[cpp_etudes](https://github.com/satanson/cpp_etudes)，可以单独下载或者保存脚本使用

试了下`calltree.pl`很好用，有点像用户态的bpftrace+funcgraph，可以通过mode：0还是1控制查看的调用栈方向

* bpftrace 用来从上到下来跟踪到指定函数，即只能看到谁调用到指定追踪点
* funcgraph 用来从指定函数往下追踪调用栈

## calltree.pl 查看C++代码的调用关系

可用于C和C++代码，用法：

```sh
# 需要用引号包含正则
calltree.pl '<src_regex>' '[[-]<dst_regex>]' <mode(0|1|2|3|4)> <verbose> <[-]depth> [[-]path_regex]

# 调用到 fsync/fdatasync 等函数的函数，并且最开始（跟层级有关）的调用者是 Write（或者部分匹配Write）
# mode：0-src_regex调用到谁 1-谁调用到src_regex 2-匹配src_regex的函数（这时候层级貌似没用） 3-
# verbose：详情好像1和0差别不大
# depth：调用层级，用3或者-3表示都行，表示只展示3层
calltree.pl '(\bfsync|fdatasync|sync_file_range\b)' 'Write'  1 1 3
# 1）调用到fsync|fdatasync|sync_file_range等函数的调用者，模式为1
# [CentOS-root@xdlinux ➜ redis git:(6.0) ✗ ]$ calltree.pl '(\bfsync|fdatasync|sync_file_range\b)' ''  1 1 3
#   (\bfsync|fdatasync|sync_file_range\b)
#   └── fsync
#       ├── rewriteAppendOnlyFile	[vim src/aof.c +1429]
#       │   └── rewriteAppendOnlyFileBackground	[vim src/aof.c +1623]
#       ├── clusterSaveConfig	[vim src/cluster.c +329]
#       │   ├── clusterSaveConfigOrDie	[vim src/cluster.c +376]
#       │   └── clusterCommand	[vim src/cluster.c +4330]

# 1）换redis的示例，谁调用到rdbSave，展示层级为3级，如果没有完整的rdbSave，则会模糊匹配
# [CentOS-root@xdlinux ➜ redis git:(6.0) ✗ ]$ calltree.pl 'rdbSave' ''  1 1 3
#   rdbSave
#   ├── flushAllDataAndResetRDB	[vim src/db.c +606]
#   │   ├── flushallCommand	[vim src/db.c +649]
#   │   └── RM_ResetDataset	[vim src/module.c +2226]
#   ├── debugCommand	[vim src/debug.c +370]
#   ├── rdbSaveBackground	[vim src/rdb.c +1386]
#   │   ├── bgsaveCommand	[vim src/rdb.c +2670]
#   │   ├── startBgsaveForReplication	[vim src/replication.c +638]
#   │   └── serverCron	[vim src/server.c +1848]
#   ├── saveCommand	[vim src/rdb.c +2655]
#   └── prepareForShutdown	[vim src/server.c +3818]
#       ├── shutdownCommand	[vim src/db.c +1045]
#       ├── serverCron	[vim src/server.c +1848]
#       └── restartServer	[vim src/server.c +2493]

# 2）模式mode为0，表示rdbSave调用到的函数关系
# [CentOS-root@xdlinux ➜ redis git:(6.0) ✗ ]$ calltree.pl 'rdbSave' ''  0 1 2
#   rdbSave	[vim src/rdb.c +1318]
#   ├── getpid
#   ├── fopen
#   ├── rioInitWithFile	[vim src/rio.c +155]
#   ├── startSaving	[vim src/rdb.c +2087]
#   │   ├── getpid
#   │   └── moduleFireServerEvent	[vim src/module.c +7377]
#   │       ├── listLength
#   │       ├── listRewind	[vim src/adlist.c +206]

# 3）加上正则表达式中，加上 (?i) 表示模糊匹配，可以用于类成员，比如(?i)CTest 会匹配 CTest::fun1, CTest::fun2
# [CentOS-root@xdlinux ➜ redis git:(6.0) ✗ ]$ calltree.pl '(?i)rdbSave' 'rdbSaveAuxField'  1 1 2 
#   (?i)rdbSave
#   ├── rdbSaveAuxField
#   │   ├── rdbSaveAuxFieldStrStr	[vim src/rdb.c +1114]
#   │   └── rdbSaveAuxFieldStrInt	[vim src/rdb.c +1119]
#   ├── rdbSaveRawString
#   │   └── rdbSaveAuxField	[vim src/rdb.c +1101]
#   └── rdbSaveType
#       └── rdbSaveAuxField	[vim src/rdb.c +1101]
```

第一次执行时，还会打印统计的行和函数个数

```sh
# 统计的仓库行数和函数个数
extract lines: 10036
function definition after merge: 814

# 第一次执行示例
[CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ calltree.pl 'fdatasync' '' 1 1 5
preprocess_all_cpp_files
register_abnormal_shutdown_hook
extract_all_funcs: begin
ag -U  -G '\.(c|cc|cpp|cu|C|h|hh|hpp|cuh|H)$'  --ignore '*test*'  --ignore '*benchmark*'  --ignore '*CMakeFiles*'  --ignore '*contrib/*'  --ignore '*third_party/*'  --ignore '*thirdparty/*'  --ignore '*3rd-[pP]arty/*'  --ignore '*3rd[pP]arty/*'  --ignore '*deps/*'  '^.*?((?:(?::{2})(?:\s)*)?(?:\b[A-Za-z_]\w*\b(?:\s)*(?::{2})(?:\s)*)*[~]?\b[A-Za-z_]\w*\b|(?:operator\s*(?:[-+*/%^&|~!=<>]=?|(?:(?:<<|>>|\|\||&&)=?)|<=>|->\*|->|\(\s*\)|\[\s*\]|\+\+|--|,)\s*(?=\()))(?:\s)*(?:\([^()]*([^()]*\((?-1)*\)[^()]*|[^()]*\([^()]*\)[^()]*)*[^()]*\))(?:(?:(?<=\))(?:\s)*:(?:\s)*(?:(?:(?:(?:(?::{2})(?:\s)*)?(?:\b[A-Za-z_]\w*\b(?:\s)*(?::{2})(?:\s)*)*[~]?\b[A-Za-z_]\w*\b(?:\s)*(?:(?:\((?:\s)*(?:(?:[^,]+?)(?:(?:\s)*,(?:\s)*(?:[^,]+?))*?)??(?:\s)*\))|(?:\{(?:\s)*(?:(?:[^,]+?)(?:(?:\s)*,(?:\s)*(?:[^,]+?))*?)??(?:\s)*\}))))(?:(?:\s)*,(?:\s)*(?:(?:(?:(?::{2})(?:\s)*)?(?:\b[A-Za-z_]\w*\b(?:\s)*(?::{2})(?:\s)*)*[~]?\b[A-Za-z_]\w*\b(?:\s)*(?:(?:\((?:\s)*(?:(?:[^,]+?)(?:(?:\s)*,(?:\s)*(?:[^,]+?))*?)??(?:\s)*\))|(?:\{(?:\s)*(?:(?:[^,]+?)(?:(?:\s)*,(?:\s)*(?:[^,]+?))*?)??(?:\s)*\})))))*?)(?:\s)*(?={)))?(?:\s)*(?:{[^{}]*([^{}]*{(?-1)*}[^{}]*|[^{}]*{[^{}]*}[^{}]*)*[^{}]*})'
extract lines: 10036
function definition after merge: 814
re_function_call=((?:((?:(?::{2})(?:\s)*)?(?:\b[A-Za-z_]\w*\b(?:\s)*(?::{2})(?:\s)*)*[~]?\b[A-Za-z_]\w*\b)(?:\s)(?:\.|->|::)(?:\s)*)?((?:(?::{2})(?:\s)*)?(?:\b[A-Za-z_]\w*\b(?:\s)*(?::{2})(?:\s)*)*[~]?\b[A-Za-z_]\w*\b))(?:\s)*[(]
process callees: begin
process callees: end
extract_all_funcs: end
  
  fdatasync
  └── SyncFd	[vim util/env_posix.cc +401]
      ├── Sync	[vim util/env_posix.cc +338]
      │   ├── BuildTable	[vim db/builder.cc +17]
      │   │   ├── DBImpl::WriteLevel0Table	[vim db/db_impl.cc +517]
      │   │   └── ConvertLogToTable	[vim db/repair.cc +143]
      │   ├── DBImpl::NewDB	[vim db/db_impl.cc +182]
...
```

统计的信息会缓存在仓库目录，可以查看`.`相关隐藏文件。删除这些文件则会再打印函数统计。

```sh
-rw-r--r--   1 root root 428K Mar 29 18:51 .calltree.summary.50.3.dat
-rw-r--r--   1 root root  310 Mar 29 18:51 .calltree.result.cached.058ad8bd954855570c6cb9ab7f36a7abc8cc095cde0003ec105f1aa4605561b8
-rw-r--r--   1 root root 4.9K Mar 29 19:01 .calltree.result.cached.812ba6cbcfbc162fb6ab919b4c965b96bf4c535a82d001e76f94479466d1c0c7
```

## cpptree.pl 查看C++代码中类的继承关系

主要用于C++代码，C中如果没有类则没啥效果

用法和实际执行示例：

```sh
./cpptree.pl <keyword|regex> <filter> <verbose(0|1)> <depth(num)>
- keyword for exact match, regex for fuzzy match;
- subtrees whose leaf nodes does not match filter are pruned, default value is '' means match all;
- verbose=0, no file locations output; otherwise succinctly output;
- depth=num, print max derivation depth.

# show all classes
./cpptree.pl '\w+'
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl '\w+'
#   \w+
#   ├── Arena
#   ├── BackgroundWorkItem
#   ├── BackgroundWorkItem
#   ├── Block

# show all classes with file locations.
# 显示所有的类，并带上文件位置
./cpptree.pl '\w+' '' 1
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl '\w+' '' 1
#   \w+
#   ├── Arena	[vim util/arena.h +16]
#   ├── BackgroundWorkItem	[vim util/env_posix.cc +767]
#   ├── BackgroundWorkItem	[vim util/env_windows.cc +685]

# show all classes exact-match ExecNode if ExecNode class exists
# 过滤指定的类，祖先是 ExecNode或者匹配ExecNode
./cpptree.pl 'ExecNode' '' 1
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl 'Iterator' '' 1
#   ^Iterator$
#   ├── Iterator	[vim db/skiplist.h +62]
#   │   ├── Block::Iter	[vim table/block.cc +77]
#   │   ├── DBIter	[vim db/db_iter.cc +39]
#   │   ├── EmptyIterator	[vim table/iterator.cc +43]
#   │   ├── MemTableIterator	[vim db/memtable.cc +46]

# show all classes fuzzy-match regex '.*Node$' if the literal class name not exists.
# 用正则匹配类
./cpptree.pl '.*Node$' '' 1
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl '.*Node$' '' 1
#   .*Node$
#   ├── CleanupNode	[vim include/leveldb/iterator.h +86]
#   └── SkipList::Node	[vim db/skiplist.h +148]

# show all classes and depth of derivation relationship is less than 3
# 继承少于3层的类
./cpptree.pl '\w+' '' 1 3
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl '\w+' '' 1 3
#   \w+
#   ├── Env	[vim include/leveldb/env.h +51]
#   │   ├── PosixEnv	[vim util/env_posix.cc +524]
#   │   ├── WindowsEnv	[vim util/env_windows.cc +383]
#   │   └── EnvWrapper	[vim include/leveldb/env.h +335]
#   │       └── InMemoryEnv	[vim helpers/memenv/memenv.cc +221]
#   └── FilterPolicy	[vim include/leveldb/filter_policy.h +27]
#       ├── BloomFilterPolicy	[vim util/bloom.cc +17]
#       ├── InternalFilterPolicy	[vim db/dbformat.h +120]
#       └── leveldb_filterpolicy_t	[vim db/c.cc +109]
#           └── Wrapper	[vim db/c.cc +473]

# show all classes whose ancestor class matches 'Node' and itself or its offsprings matches 'Scan'
# 类的祖先是 Node，且自己匹配 Scan
/cpptree.pl 'Node' 'Scan'
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl 'FilterPolicy' 'leveldb_filterpolicy_t'
# 结果里会显示是全词匹配到，还是部分匹配
#   ^FilterPolicy$
#   └── FilterPolicy
#       └── leveldb_filterpolicy_t

# 这里就是部分匹配到，下面没有^和$
# [CentOS-root@xdlinux ➜ leveldb git:(main) ✗ ]$ cpptree.pl 'FilterP' 'b_filterp' 
#   FilterP
#   └── FilterPolicy
#       └── leveldb_filterpolicy_t
```

## csvtable.pl，可以把csv文件转换成表格

示例：

```sh
cat >repeat.csv  <<'DONE'
t,vectorization,new-vectorization,non-vectorization,new speedup,vectorization speedup
repeat/const_times,3112ms, 9797ms, 13986ms(4X memory),3.14,4.49
repeat/const_s, 8107ms, 30231ms, 39501ms(32X memory), 3.72,4.87
repeat,9141ms, 32182ms,44315ms(32X memory), 3.52,4.84
DONE

# 转换
cat repeat.csv |./csvtable.pl

+====================+===============+===================+=====================+=============+=======================+
| test               | vectorization | new-vectorization | non-vectorization   | new speedup | vectorization speedup |
+====================+===============+===================+=====================+=============+=======================+
| repeat/const_times | 3112ms        | 9797ms            | 13986ms(4X memory)  | 3.14        | 4.49                  |
| repeat/const_s     | 8107ms        | 30231ms           | 39501ms(32X memory) | 3.72        | 4.87                  |
| repeat             | 9141ms        | 32182ms           | 44315ms(32X memory) | 3.52        | 4.84                  |
+--------------------+---------------+-------------------+---------------------+-------------+-----------------------+
```
