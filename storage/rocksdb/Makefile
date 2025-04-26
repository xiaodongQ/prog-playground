MODE ?= debug
INCLUDE_PATH = /usr/local/rocksdb/include

# 进一步支持debug/release模式区分
ifeq ($(MODE), release)
	CXX_FLAGS = -O2 -g -Wall
else ifeq ($(MODE), debug)
	CXX_FLAGS = -g -Wall
else
	$(error invalid MODE:$(MODE))
endif

# =========== begin ==============
# gcc默认情况下，会优先选择动态库
# 链接静态库方式1：
# -Wl,-Bstatic 显式指定链接静态库，但注意会影响其他库，它后面的所有库都是静态方式链接。需要 -Wl,-Bdynamic 恢复默认行为
#LD_FLAGS = -Wl,-Bstatic -lrocksdb -L/usr/local/rocksdb/lib64 -Wl,-Bdynamic -pthread

# 链接静态库方式2：直接指定静态库路径，显式指定.a，不用-l（绕过了动态库搜索机制）
LD_FLAGS = /usr/local/rocksdb/lib64/librocksdb.a -pthread
# =========== end ==============

TARGET = test_rocksdb_ops
SRCS = test_rocksdb_ops.cpp
# 方式1
# OBJS = $(SRCS:.cpp=.o)
# 方式2
OBJS = $(patsubst %.cpp, %.o, $(SRCS))

# 默认目标
all: $(TARGET)
#all:
#g++ test_rocksdb_ops.cpp -o test_rocksdb_ops -I/usr/local/rocksdb/include ${LD_FLAGS}

# 生成可执行文件
$(TARGET): $(OBJS)
	g++ $(OBJS) -o $@ -I$(INCLUDE_PATH) $(LD_FLAGS)
ifeq ($(MODE), release)
	objcopy --only-keep-debug $@ $@.debug
# 和 strip --strip-debug 效果相当，移除调试信息但保留符号表，strip --strip-all（默认模式）则移除所有符号表和调试信息，大小显著减小
	objcopy --strip-debug $@
	objcopy --add-gnu-debuglink=$@.debug $@
endif

# 生成目标文件
%.o:%.cpp
	g++ $(CXX_FLAGS) -c $< -o $@ -I$(INCLUDE_PATH)

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).debug

.PHONY: all clean
