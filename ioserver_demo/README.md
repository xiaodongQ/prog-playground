# 读写服务端demo项目

说明：基于C++实现的读写服务demo，借此作为场景温习并深入学习io多路复用、性能调试、开源组件。

借助 [trae](https://traeide.com/zh/) 的`Builder`模式快速搭建项目。

## 涉及技术点

学习涉及的一些思路

1、阶段一：

* io复用select/poll/epoll
* 线程池、内存池、
* 缓存Redis、MySQL
* 性能定位和调试：火焰图、上下文切换、cpu亲和性设置、tcp参数调优
* 容器化，dockerfile
* 监测：Prometheus、Grafana，各项指标设计采集
    - 网络，带宽，tcp丢包、重传、rt等指标
    - 内存使用
    - 磁盘io使用率，带宽
    - 服务端的连接数
    * 考虑如何结合`ebpf`进行监测，如何设计指标

2、阶段二：分布式服务

* 服务端的负载均衡，reuseport、api网关
* redis集群
* grpc框架
* 服务发现etcd/consoul
* 对称式架构，raft选举

3、阶段三：结合场景实践一些算法和机制方案等

* 一致性hash、分布式锁
* 布隆过滤器，过滤异常请求
* 备份、容灾

