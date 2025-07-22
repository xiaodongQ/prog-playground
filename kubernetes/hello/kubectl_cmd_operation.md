# kubectl相关命令操作结果记录

## kubectl describe node

[root@xdlinux ➜ hello git:(main) ]$ kubectl describe node xdlinux
Name:               xdlinux
Roles:              control-plane
Labels:             beta.kubernetes.io/arch=amd64
                    beta.kubernetes.io/os=linux
                    kubernetes.io/arch=amd64
                    kubernetes.io/hostname=xdlinux
                    kubernetes.io/os=linux
                    node-role.kubernetes.io/control-plane=
                    node.kubernetes.io/exclude-from-external-load-balancers=
Annotations:        kubeadm.alpha.kubernetes.io/cri-socket: unix:///var/run/containerd/containerd.sock
                    node.alpha.kubernetes.io/ttl: 0
                    volumes.kubernetes.io/controller-managed-attach-detach: true
CreationTimestamp:  Mon, 21 Jul 2025 07:47:52 +0800
Taints:             node-role.kubernetes.io/control-plane:NoSchedule
Unschedulable:      false
Lease:
  HolderIdentity:  xdlinux
  AcquireTime:     <unset>
  RenewTime:       Tue, 22 Jul 2025 22:57:36 +0800
Conditions:
  Type             Status  LastHeartbeatTime                 LastTransitionTime                Reason                       Message
  ----             ------  -----------------                 ------------------                ------                       -------
  MemoryPressure   False   Tue, 22 Jul 2025 22:57:34 +0800   Mon, 21 Jul 2025 07:47:51 +0800   KubeletHasSufficientMemory   kubelet has sufficient memory available
  DiskPressure     False   Tue, 22 Jul 2025 22:57:34 +0800   Mon, 21 Jul 2025 07:47:51 +0800   KubeletHasNoDiskPressure     kubelet has no disk pressure
  PIDPressure      False   Tue, 22 Jul 2025 22:57:34 +0800   Mon, 21 Jul 2025 07:47:51 +0800   KubeletHasSufficientPID      kubelet has sufficient PID available
  Ready            True    Tue, 22 Jul 2025 22:57:34 +0800   Tue, 22 Jul 2025 20:08:06 +0800   KubeletReady                 kubelet is posting ready status
Addresses:
  InternalIP:  192.168.1.150
  Hostname:    xdlinux
Capacity:
  cpu:                16
  ephemeral-storage:  56872Mi
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             31929508Ki
  pods:               110
Allocatable:
  cpu:                16
  ephemeral-storage:  53671152756
  hugepages-1Gi:      0
  hugepages-2Mi:      0
  memory:             31827108Ki
  pods:               110
System Info:
  Machine ID:                 fedf1414bf654cf090067b7374c2a61a
  System UUID:                3adbd480-702c-11ec-8165-1ad318f15100
  Boot ID:                    28911b0d-93b1-4ba3-9f90-4d3c61270498
  Kernel Version:             5.14.0-503.40.1.el9_5.x86_64
  OS Image:                   Rocky Linux 9.5 (Blue Onyx)
  Operating System:           linux
  Architecture:               amd64
  Container Runtime Version:  containerd://2.1.3
  Kubelet Version:            v1.33.3
  Kube-Proxy Version:         
PodCIDR:                      10.244.0.0/24
PodCIDRs:                     10.244.0.0/24
Non-terminated Pods:          (7 in total)
  Namespace                   Name                               CPU Requests  CPU Limits  Memory Requests  Memory Limits  Age
  ---------                   ----                               ------------  ----------  ---------------  -------------  ---
  kube-system                 coredns-757cc6c8f8-nxw7g           100m (0%)     0 (0%)      70Mi (0%)        170Mi (0%)     39h
  kube-system                 coredns-757cc6c8f8-v2mgf           100m (0%)     0 (0%)      70Mi (0%)        170Mi (0%)     39h
  kube-system                 etcd-xdlinux                       100m (0%)     0 (0%)      100Mi (0%)       0 (0%)         39h
  kube-system                 kube-apiserver-xdlinux             250m (1%)     0 (0%)      0 (0%)           0 (0%)         39h
  kube-system                 kube-controller-manager-xdlinux    200m (1%)     0 (0%)      0 (0%)           0 (0%)         39h
  kube-system                 kube-proxy-v682x                   0 (0%)        0 (0%)      0 (0%)           0 (0%)         39h
  kube-system                 kube-scheduler-xdlinux             100m (0%)     0 (0%)      0 (0%)           0 (0%)         39h
Allocated resources:
  (Total limits may be over 100 percent, i.e., overcommitted.)
  Resource           Requests    Limits
  --------           --------    ------
  cpu                850m (5%)   0 (0%)
  memory             240Mi (0%)  340Mi (1%)
  ephemeral-storage  0 (0%)      0 (0%)
  hugepages-1Gi      0 (0%)      0 (0%)
  hugepages-2Mi      0 (0%)      0 (0%)
Events:              <none>
