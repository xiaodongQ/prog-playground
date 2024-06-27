#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_tracing.h>

#include "test_tcptrace.h"


char LICENSE[] SEC("license") = "Dual BSD/GPL";

// 转换为libbpf方式的事件ring buffer，key可以传一个指针
// BPF_PERF_OUTPUT(ipv4_events);
struct {
 __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
 __uint(key_size, sizeof(__u32));
 __uint(value_size, sizeof(__u32));
} ipv4_events SEC(".maps");

static struct tcphdr *skb_to_tcphdr(const struct sk_buff *skb)
{
    // unstable API. verify logic in tcp_hdr() -> skb_transport_header().
    return (struct tcphdr *)(skb->head + skb->transport_header);
}

static inline struct iphdr *skb_to_iphdr(const struct sk_buff *skb)
{
    // unstable API. verify logic in ip_hdr() -> skb_network_header().
    return (struct iphdr *)(skb->head + skb->network_header);
}


// 到/sys/kernel/debug/tracing/available_events里找下已有的tcp相关tracepoint，没有太合适的
// 参考bcc tools的tcpdrop，用 skb:kfree_skb 跟踪点，搜对应vmlinux.h，找到上下文trace_event_raw_kfree_skb
 // 不同vmlinux.h可能有差异，可以生成当前内核匹配的文件：bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
SEC("tracepoint/skb/kfree_skb")
int handle_tp(struct trace_event_raw_kfree_skb *args)
{
 struct sk_buff *skb = (struct sk_buff*)BPF_CORE_READ(args, skbaddr);
 struct sock *sk = skb->sk;

 if (sk == NULL)
        return 0;
    __u32 pid = bpf_get_current_pid_tgid() >> 32;

    // pull in details from the packet headers and the sock struct
    __u16 family = sk->__sk_common.skc_family;
    char state = sk->__sk_common.skc_state;
    __u16 sport = 0, dport = 0;
    struct tcphdr *tcp = skb_to_tcphdr(skb);
    struct iphdr *ip = skb_to_iphdr(skb);
    __u8 tcpflags = ((__u8 *)tcp)[13];
    sport = tcp->source;
    dport = tcp->dest;

 // 只管 IPV4，暂不考虑IPV6
    if (family == AF_INET) {
        struct ipv4_data_t data4 = {};
        data4.pid = pid;
        data4.ip = 4;
        data4.saddr = ip->saddr;
        data4.daddr = ip->daddr;
        data4.dport = dport;
        data4.sport = sport;
        data4.state = state;
        data4.tcpflags = tcpflags;
  // 这里的 调用栈 是BCC里实现的宏，对应BPF_MAP_TYPE_STACK，实现调用栈功能有点复杂，暂不用
        // data4.stack_id = stack_traces.get_stackid(args, 0);
  // 换成libbpf方式操作性能事件
        // ipv4_events.perf_submit(args, &data4, sizeof(data4));
  bpf_perf_event_output(args, &ipv4_events, BPF_F_CURRENT_CPU, &data4, sizeof(data4));
    }
    // else drop

    return 0;
}


/*
对于上面vmlinux.h在不同内核的区别，这里有个直观的例子
1、libbpf-bootstrap-master\vmlinux\x86\vmlinux_601.h：
struct trace_event_raw_kfree_skb {
 struct trace_entry ent;
 void *skbaddr;
 void *location;
 short unsigned int protocol;
 enum skb_drop_reason reason;
 char __data[0];
};
2、从环境里用bpftool重新生成的vmlinux.h
struct trace_event_raw_kfree_skb {
 struct trace_entry ent;
 void *skbaddr;
 void *location;
 short unsigned int protocol;
 char __data[0];
};
3、新内核加了个丢包理由 enum skb_drop_reason reason;
而当前环境A里是不支持的。这也解释了用tcpdrop时，在环境A没抓到drop的原因
这也体现了libbpf CO-RE的价值，一次编译到处运行
---

bcc-master\tools\tcpdrop.py里面，使用到了reason：

bpf_kfree_skb_text = """
#include <linux/skbuff.h>

TRACEPOINT_PROBE(skb, kfree_skb) {
    struct sk_buff *skb = args->skbaddr;
    struct sock *sk = skb->sk;
    enum skb_drop_reason reason = args->reason;

    // SKB_NOT_DROPPED_YET,
    // SKB_DROP_REASON_NOT_SPECIFIED,
    if (reason > SKB_DROP_REASON_NOT_SPECIFIED) {
        return __trace_tcp_drop(args, sk, skb);
    }

    return 0;
}
"""
*/
