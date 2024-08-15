CC=g++
CFLAGS=-std=c++11 -Wall -Wextra

all: write_tempfile

write_tempfile: write_by_signal.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f write_tempfile
