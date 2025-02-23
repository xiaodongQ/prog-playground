# 编译器和编译选项
CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++14 -I./include

# 检查系统依赖
CHECK_MYSQL := $(shell pkg-config --exists mysqlclient && echo "yes" || echo "no")
CHECK_REDIS := $(shell pkg-config --exists hiredis && echo "yes" || echo "no")

# 验证依赖是否满足
ifeq ($(CHECK_MYSQL),no)
$(error MySQL client library not found. Please install libmysqlclient-dev)
endif

ifeq ($(CHECK_REDIS),no)
$(error Redis library not found. Please install libhiredis-dev)
endif

# 获取依赖的编译和链接标志
MYSQL_CFLAGS := $(shell pkg-config --cflags mysqlclient)
MYSQL_LIBS := $(shell pkg-config --libs mysqlclient)
REDIS_CFLAGS := $(shell pkg-config --cflags hiredis)
REDIS_LIBS := $(shell pkg-config --libs hiredis)

# 更新CFLAGS，加入依赖的编译标志
CFLAGS += $(MYSQL_CFLAGS) $(REDIS_CFLAGS)

# 目标目录
BIN_DIR = bin
OBJ_DIR = obj
SRC_DIR = src
INCLUDE_DIR = include

# 创建目标目录
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR))

# 源文件和目标文件
SERVER_SRCS = $(wildcard $(SRC_DIR)/server/*.cpp)
CLIENT_SRCS = $(wildcard $(SRC_DIR)/client/*.cpp)
COMMON_SRCS = $(wildcard $(SRC_DIR)/common/*.cpp)

SERVER_OBJS = $(patsubst $(SRC_DIR)/server/%.cpp,$(OBJ_DIR)/server/%.o,$(SERVER_SRCS))
CLIENT_OBJS = $(patsubst $(SRC_DIR)/client/%.cpp,$(OBJ_DIR)/client/%.o,$(CLIENT_SRCS))
COMMON_OBJS = $(patsubst $(SRC_DIR)/common/%.cpp,$(OBJ_DIR)/common/%.o,$(COMMON_SRCS))

# 目标文件
TARGETS = $(BIN_DIR)/server $(BIN_DIR)/client

# 默认目标：构建所有目标
all: prepare $(TARGETS)

# 准备目录结构
prepare:
	mkdir -p $(OBJ_DIR)/server $(OBJ_DIR)/client $(OBJ_DIR)/common

# 规则：编译server
$(BIN_DIR)/server: $(SERVER_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread $(MYSQL_LIBS) $(REDIS_LIBS)

# 规则：编译client
$(BIN_DIR)/client: $(CLIENT_OBJS) $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# 编译server的.cpp文件
$(OBJ_DIR)/server/%.o: $(SRC_DIR)/server/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 编译client的.cpp文件
$(OBJ_DIR)/client/%.o: $(SRC_DIR)/client/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 编译common的.cpp文件
$(OBJ_DIR)/common/%.o: $(SRC_DIR)/common/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

# 重新构建
rebuild: clean all

# 检查依赖
check-deps:
	@echo "Checking dependencies..."
	@echo "MySQL client library: $(CHECK_MYSQL)"
	@echo "Redis library: $(CHECK_REDIS)"

.PHONY: all clean rebuild prepare check-deps