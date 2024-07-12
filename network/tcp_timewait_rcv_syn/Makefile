# 编译器和编译选项
CC = g++
CFLAGS = -Wextra -pedantic -std=c++11

# 目标文件
TARGETS = server

# 源文件和对象文件
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# 默认目标：构建所有目标
all: $(TARGETS)

# 规则：编译client
client: client.o
	$(CC) $(CFLAGS) -lpthread -o $@ $^

# 规则：编译server
server: server.o
	$(CC) $(CFLAGS) -o $@ $^

# .cpp到.o的通用规则
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGETS)

# 重新构建（清理并重建所有）
rebuild: clean all

.PHONY: all clean rebuild
