/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
/* Copyright (c) 2022 Meta Platforms, Inc. */
#ifndef __TEST_TCPTRACE_H_
#define __TEST_TCPTRACE_H_

#define AF_INET    2
#define AF_INET6   10

// 作为丢包事件结构
struct ipv4_data_t {
    __u32 pid;
    __u64 ip;
    __u32 saddr;
    __u32 daddr;
    __u16 sport;
    __u16 dport;
    __u8 state;
    __u8 tcpflags;
    __u32 stack_id;
};

#endif /* __TEST_TCPTRACE_H_ */
