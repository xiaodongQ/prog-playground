#!/bin/bash

g++ server.cpp -o server
g++ client.cpp -o client -std=c++11 -lpthread
