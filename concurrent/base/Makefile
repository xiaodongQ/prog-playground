CXX =  g++
CXXFLAGS = -std=c++11 -pthread -g -Wall
# 使用gperftools
CXXFLAGS += -lprofiler

TARGET = thread_pool_withwait
SRCS = thread_pool_withwait.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

