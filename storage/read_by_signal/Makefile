# Makefile for compiling read_by_signal.cpp into read_fstab executable

CC=g++
CFLAGS=-std=c++11 -Wall -Wextra

all: read_fstab

read_fstab: read_by_signal.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f read_fstab
