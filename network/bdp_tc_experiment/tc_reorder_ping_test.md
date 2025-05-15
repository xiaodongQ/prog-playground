### tc reorder

> 博客记录可见：[TCP发送接收过程（三） -- 实验观察TCP性能和窗口、Buffer的关系（下）](https://xiaodongq.github.io/2025/05/12/tcp-window-size-case-tc/)

reorder和delay的组合有2种写法：

* `tc qdisc add dev eth0 root netem reorder 80% delay 50ms`
* `tc qdisc add dev eth0 root netem delay 50ms reorder 80%`

delay的位置不同会有不同效果吗？

#### 3.2.2. reorder和delay先后顺序

实验方式：在服务端增加`reorder`对应qdisc，并在客户端ping进行观察。

**先贴下实验结论：**

| 用例                                                    | 结果                         | ping表现                       |
| ------------------------------------------------------- | ---------------------------- | ------------------------------ |
| 1、reorder 80% delay 50ms                               | delay 50ms reorder 80% gap 1 | 接近20%概率延迟（10次出现2次） |
| 2、delay 50ms reorder 80%                               | delay 50ms reorder 80% gap 1 | 表现同上                       |
| 3、reorder 80% gap 5 delay 50ms (gap 5放最后面效果一样) | delay 50ms reorder 80% gap 5 | 5次出现4次延迟，1次不延迟      |
| 4、delay 50ms reorder 80% gap 5                         | delay 50ms reorder 80% gap 5 | 同上                           |
| 5、reorder 80% gap 8 delay 50ms                         | delay 50ms reorder 80% gap 8 | 8次出现7次延迟，1次不延迟      |

说明：
* 上述表格，用例列中省略了共同的`tc qdisc add dev eth0 root netem`；结果列中则省略了`qdisc netem 800d: root refcnt 2 limit 1000`
* 可看到不管`reorder`和`delay`的顺序如何，表现相同。并不会出现LLM说的基础延迟再叠加乱序包延迟的情况（乱序包2倍延迟）。
* `gap`非默认值1时，近似会出现固定的`gap-1`次延迟，剩余一次则根据设置的比例作为非延迟的概率（比例表示立即发送的概率）

---

1、case1：delay在reorder后面，gap默认1

``sh
# case1、服务端添加规则1：delay在reorder后面
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc add dev eth0 root netem reorder 80% delay 50ms
# 查看
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc netem 800d: root refcnt 2 limit 1000 delay 50ms reorder 80% gap 1
``

客户端ping，出现延迟50ms概率是低的，接近20%（10次里有2次）

``sh
[root@iZbp1dkjrwztxfyf6vufruZ ~]# ping 172.16.58.147
PING 172.16.58.147 (172.16.58.147) 56(84) bytes of data.
64 bytes from 172.16.58.147: icmp_seq=1 ttl=64 time=0.282 ms
64 bytes from 172.16.58.147: icmp_seq=2 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=3 ttl=64 time=0.226 ms
64 bytes from 172.16.58.147: icmp_seq=4 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=5 ttl=64 time=0.215 ms
64 bytes from 172.16.58.147: icmp_seq=6 ttl=64 time=0.230 ms
64 bytes from 172.16.58.147: icmp_seq=7 ttl=64 time=0.234 ms
64 bytes from 172.16.58.147: icmp_seq=8 ttl=64 time=0.235 ms
64 bytes from 172.16.58.147: icmp_seq=9 ttl=64 time=0.244 ms
64 bytes from 172.16.58.147: icmp_seq=10 ttl=64 time=0.250 ms
64 bytes from 172.16.58.147: icmp_seq=11 ttl=64 time=0.220 ms
64 bytes from 172.16.58.147: icmp_seq=12 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=13 ttl=64 time=0.230 ms
64 bytes from 172.16.58.147: icmp_seq=14 ttl=64 time=0.189 ms
64 bytes from 172.16.58.147: icmp_seq=15 ttl=64 time=0.238 ms
64 bytes from 172.16.58.147: icmp_seq=16 ttl=64 time=0.257 ms
64 bytes from 172.16.58.147: icmp_seq=17 ttl=64 time=0.244 ms
``

2、case2：delay在reorder前面，gap默认1

``sh
# 清理规则
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc del dev eth0 root
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc fq_codel 0: root refcnt 2 limit 10240p flows 1024 quantum 1514 target 5ms interval 100ms memory_limit 32Mb ecn drop_batch 64

# case2、服务端添加规则2：delay在reorder前面
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc add dev eth0 root netem delay 50ms reorder 80%
# 查看
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc netem 800e: root refcnt 2 limit 1000 delay 50ms reorder 80% gap 1
``

客户端ping，出现延迟50ms概率是低的，接近20%

``sh
[root@iZbp1dkjrwztxfyf6vufruZ ~]# ping 172.16.58.147
PING 172.16.58.147 (172.16.58.147) 56(84) bytes of data.
64 bytes from 172.16.58.147: icmp_seq=1 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=2 ttl=64 time=0.231 ms
64 bytes from 172.16.58.147: icmp_seq=3 ttl=64 time=0.225 ms
64 bytes from 172.16.58.147: icmp_seq=4 ttl=64 time=0.259 ms
64 bytes from 172.16.58.147: icmp_seq=5 ttl=64 time=0.231 ms
64 bytes from 172.16.58.147: icmp_seq=6 ttl=64 time=0.224 ms
64 bytes from 172.16.58.147: icmp_seq=7 ttl=64 time=0.231 ms
64 bytes from 172.16.58.147: icmp_seq=8 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=9 ttl=64 time=0.228 ms
64 bytes from 172.16.58.147: icmp_seq=10 ttl=64 time=0.246 ms
64 bytes from 172.16.58.147: icmp_seq=11 ttl=64 time=0.223 ms
64 bytes from 172.16.58.147: icmp_seq=12 ttl=64 time=0.275 ms
64 bytes from 172.16.58.147: icmp_seq=13 ttl=64 time=0.235 ms
64 bytes from 172.16.58.147: icmp_seq=14 ttl=64 time=0.256 ms
``

3、case3：delay在reorder后面，gap 5

``sh
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc del dev eth0 root
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc add dev eth0 root netem reorder 80% gap 5 delay 50ms
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc netem 8011: root refcnt 2 limit 1000 delay 50ms reorder 80% gap 5
``

客户端看，基本是3~4个50ms延迟后（和`gap-1`对应），而后出现不延迟。按5个icmp看，有1次不延迟（20%几率不延迟，即80%几率延迟）
* tc规则中的`80%`表示剩下那个包有80%几率立即发送或20%几率延迟发送

``sh
[root@iZbp1dkjrwztxfyf6vufruZ ~]# ping 172.16.58.147
PING 172.16.58.147 (172.16.58.147) 56(84) bytes of data.
64 bytes from 172.16.58.147: icmp_seq=1 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=2 ttl=64 time=0.215 ms
64 bytes from 172.16.58.147: icmp_seq=3 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=4 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=5 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=6 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=7 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=8 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=9 ttl=64 time=0.216 ms
64 bytes from 172.16.58.147: icmp_seq=10 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=11 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=12 ttl=64 time=0.213 ms
64 bytes from 172.16.58.147: icmp_seq=13 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=14 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=15 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=16 ttl=64 time=50.2 ms
``

4、case4：delay在reorder前面，gap 5

``sh
# 清理规则
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc del dev eth0 root
# 添加规则
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc add dev eth0 root netem delay 50ms reorder 80% gap 5
# 查看
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc netem 800f: root refcnt 2 limit 1000 delay 50ms reorder 80% gap 5
``

客户端看，基本是3~4个50ms延迟后（和`gap-1`对应），出现不延迟。按5个icmp看，有1次不延迟（20%几率不延迟，即80%几率延迟）。
* **且不会出现包有`100ms`延迟**。

``sh
[root@iZbp1dkjrwztxfyf6vufruZ ~]# ping 172.16.58.147
PING 172.16.58.147 (172.16.58.147) 56(84) bytes of data.
64 bytes from 172.16.58.147: icmp_seq=1 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=2 ttl=64 time=0.215 ms
64 bytes from 172.16.58.147: icmp_seq=3 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=4 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=5 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=6 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=7 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=8 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=9 ttl=64 time=0.202 ms
64 bytes from 172.16.58.147: icmp_seq=10 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=11 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=12 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=13 ttl=64 time=0.233 ms
64 bytes from 172.16.58.147: icmp_seq=14 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=15 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=16 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=17 ttl=64 time=0.207 ms
64 bytes from 172.16.58.147: icmp_seq=18 ttl=64 time=50.3 ms
``

5、`tc qdisc add dev eth0 root netem reorder 80% gap 8 delay 50ms`

8次出现7次延迟，1次不延迟。

``sh
[root@iZbp1dkjrwztxfyf6vufruZ ~]# ping 172.16.58.147
PING 172.16.58.147 (172.16.58.147) 56(84) bytes of data.
64 bytes from 172.16.58.147: icmp_seq=1 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=2 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=3 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=4 ttl=64 time=0.227 ms
64 bytes from 172.16.58.147: icmp_seq=5 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=6 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=7 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=8 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=9 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=10 ttl=64 time=0.226 ms
64 bytes from 172.16.58.147: icmp_seq=11 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=12 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=13 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=14 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=15 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=16 ttl=64 time=0.220 ms
64 bytes from 172.16.58.147: icmp_seq=17 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=18 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=19 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=20 ttl=64 time=50.2 ms
64 bytes from 172.16.58.147: icmp_seq=21 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=22 ttl=64 time=50.3 ms
64 bytes from 172.16.58.147: icmp_seq=23 ttl=64 time=0.225 ms
64 bytes from 172.16.58.147: icmp_seq=24 ttl=64 time=50.2 ms
```
