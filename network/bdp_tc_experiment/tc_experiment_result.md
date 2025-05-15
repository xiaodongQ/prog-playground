# tc实验结果

用例情况如下：

1. 客户端延时 2ms
2. 客户端延时 100ms
3. 服务端丢包 1%
4. 服务端丢包 10%
5. 服务端包重复 1%
6. 服务端包重复 10%
7. 服务端包损坏 1%
8. 服务端包损坏 10%
9. 服务端包乱序 1%
10. 服务端包乱序 20%
11. 限制带宽 400Mbps (50MBps)
12. 限制带宽 8Mbps (1MBps)


```sh
# case1
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0   108M      0  0:00:18  0:00:18 --:--:--  103M

# case2
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0  34.2M      0  0:00:59  0:00:59 --:--:-- 34.0M

# case3
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0   107M      0  0:00:19  0:00:19 --:--:--  103M

# case4 打断
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
  1 2048M    1 26.0M    0     0   858k      0  0:40:42  0:00:31  0:40:11 1544k

# case5
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0   110M      0  0:00:18  0:00:18 --:--:--  103M

# case6
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0   108M      0  0:00:18  0:00:18 --:--:--  104M

# case7
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0   110M      0  0:00:18  0:00:18 --:--:--  103M

# case8 打断
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
  3 2048M    3 75.7M    0     0  1982k      0  0:17:37  0:00:39  0:16:58 1528k

# case9
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0   113M      0  0:00:18  0:00:18 --:--:--  103M

# case10 打断
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
  0 2048M    0 7114k    0     0   194k      0  2:59:15  0:00:36  2:58:39  198k

# case11
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100 2048M  100 2048M    0     0  45.6M      0  0:00:44  0:00:44 --:--:-- 45.6M
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc tbf 801a: root refcnt 2 rate 400Mbit burst 1310700b lat 10ms

# case12 打断
[root@iZbp1dkjrwztxfyf6vufruZ ~]# curl 172.16.58.147:8000/test.dat -O
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
  1 2048M    1 33.2M    0     0   934k      0  0:37:24  0:00:36  0:36:48  933k
[root@iZbp1dkjrwztxfyf6vufrvZ ~]# tc qdisc show dev eth0 root
qdisc tbf 801b: root refcnt 2 rate 8Mbit burst 25Kb lat 10ms
```
