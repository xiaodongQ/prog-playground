CC=g++
CFLAGS=-std=c++11 -Wall -Wextra
TARGET=write_direct

all: $(TARGET)

$(TARGET): write_by_signal_direct.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
