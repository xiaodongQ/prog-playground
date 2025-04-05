# 说明

该文件是 Doug Lea（道格.利）实现的malloc（dlmalloc）代码，最开始的版本起始于 Aug 28 13:14:29 1992。glibc的ptmalloc基于这版改进后支持多线程。

下载代码供对比学习。

* dlmalloc介绍：https://gee.cs.oswego.edu/dl/html/malloc.html
* 代码：https://gee.cs.oswego.edu/pub/misc/malloc.c

## 使用

这个库只有一个文件，下载代码后，编译（-O3优化），并链接到其他程序。

单独编译，则可： gcc -c malloc.c -o malloc.o
