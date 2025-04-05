# 编译器和标志
CC = g++
CFLAGS = -Wall -g
LDFLAGS = -lpthread

# 程序名称和源文件
TARGET = leak_test
SRCS = leak_test.cpp

# 默认目标
all: $(TARGET)

# 编译普通版本
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

# 使用 AddressSanitizer 编译
# 若要检测到报错后支持继续执行，需要加`-fsanitize-recover=address`，并且运行时设置`ASAN_OPTIONS=halt_on_error=0`
asan: CFLAGS += -fsanitize=address -fsanitize-recover=address
asan: CFLAGS += -DTEST_LEAK
asan: clean $(TARGET)

# 单独使用 LeakSanitizer 编译 (通常与 AddressSanitizer 一起启用)
# AddressSanitizer里面已经默认集成了LeakSanitizer，asan不需要显式指定
lsan: CFLAGS += -fsanitize=leak
lsan: CFLAGS += -DTEST_LEAK
lsan: clean $(TARGET)

# MemorySanitizer
msan: CFLAGS += -fsanitize=memory
msan: clean $(TARGET)

# 使用 UndefinedBehaviorSanitizer 编译
ubsan: CFLAGS += -fsanitize=undefined
ubsan: clean $(TARGET)

# ThreadSanitizer
tsan: CFLAGS += -fsanitize=thread
tsan: clean $(TARGET)

# 清理生成的文件
clean:
	rm -f $(TARGET)

.PHONY: all asan lsan ubsan clean
