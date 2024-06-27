// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "test_tcptrace.skel.h"
#include "test_tcptrace.h"

#define PERF_BUFFER_PAGES 16
#define PERF_POLL_TIMEOUT_MS 100

static const char *tcp_states[] = {
    [1] = "ESTABLISHED",
    [2] = "SYN_SENT",
    [3] = "SYN_RECV",
    [4] = "FIN_WAIT1",
    [5] = "FIN_WAIT2",
    [6] = "TIME_WAIT",
    [7] = "CLOSE",
    [8] = "CLOSE_WAIT",
    [9] = "LAST_ACK",
    [10] = "LISTEN",
    [11] = "CLOSING",
    [12] = "NEW_SYN_RECV",
    [13] = "UNKNOWN",
};

#define TCPHDR_FIN 0x01
#define TCPHDR_SYN 0x02
#define TCPHDR_RST 0x04
#define TCPHDR_PSH 0x08
#define TCPHDR_ACK 0x10
#define TCPHDR_URG 0x20
#define TCPHDR_ECE 0x40
#define TCPHDR_CWR 0x80

void flags2str(unsigned char flags, char *result) {
    char* arr[8];
    int count = 0;
    
    if (flags & TCPHDR_FIN) {
        arr[count] = "FIN";
        count++;
    }
    if (flags & TCPHDR_SYN) {
        arr[count] = "SYN";
        count++;
    }
    if (flags & TCPHDR_RST) {
        arr[count] = "RST";
        count++;
    }
    if (flags & TCPHDR_PSH) {
        arr[count] = "PSH";
        count++;
    }
    if (flags & TCPHDR_ACK) {
        arr[count] = "ACK";
        count++;
    }
    if (flags & TCPHDR_URG) {
        arr[count] = "URG";
        count++;
    }
    if (flags & TCPHDR_ECE) {
        arr[count] = "ECE";
        count++;
    }
    if (flags & TCPHDR_CWR) {
        arr[count] = "CWR";
        count++;
    }
    
    strcpy(result, "");
    
    for (int i = 0; i < count; i++) {
        strncat(result, arr[i], strlen(arr[i]));
        if (i != count - 1) {
            strcat(result, "|");
        }
    }
    
    return;
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args) {
    return vfprintf(stderr, format, args);
}

static void handle_event(void *ctx, int cpu, void *data, __u32 data_sz) {
    char ts[32], saddr[48], daddr[48];
    struct ipv4_data_t e;
    struct tm *tm;
    time_t t;

    if (data_sz < sizeof(e)) {
        printf("Error: packet too small\n");
        return;
    }
    /* Copy data as alignment in the perf buffer isn't guaranteed. */
    memcpy(&e, data, sizeof(e));

    inet_ntop(AF_INET, &e.saddr, saddr, sizeof(saddr));
    inet_ntop(AF_INET, &e.daddr, daddr, sizeof(daddr));
    char tcp_flag_buff[32] = {0};
    flags2str(e.tcpflags, tcp_flag_buff);

    char sbuff[64] = {0};
    char dbuff[64] = {0};
    snprintf(sbuff, sizeof(sbuff), "%s:%d", saddr, ntohs(e.sport));
    snprintf(dbuff, sizeof(dbuff), "%s:%d", daddr, ntohs(e.dport));
    
    printf("%-8s %-7d %-2d %-20s > %-20s %s (%s)",
           "xxx", e.pid, e.ip,
           sbuff,
           dbuff,
           tcp_states[e.state], tcp_flag_buff);
}

static void handle_lost_events(void *ctx, int cpu, __u64 lost_cnt) {
    fprintf(stderr, "lost %llu events on CPU #%d\n", lost_cnt, cpu);
}

int main(int argc, char **argv) {
    struct test_tcptrace_bpf *skel;
    struct perf_buffer *pb = NULL;
    int err;

    /* Set up libbpf errors and debug info callback */
    libbpf_set_print(libbpf_print_fn);

    /* Open BPF application */
    skel = test_tcptrace_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }

    /* Load & verify BPF programs */
    err = test_tcptrace_bpf__load(skel);
    if (err) {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        goto cleanup;
    }

    /* Attach tracepoint handler */
    err = test_tcptrace_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        goto cleanup;
    }

    pb = perf_buffer__new(bpf_map__fd(skel->maps.ipv4_events), PERF_BUFFER_PAGES,
                          handle_event, handle_lost_events, NULL, NULL);
    if (!pb) {
        err = -errno;
        fprintf(stderr, "failed to open perf buffer: %d\n", err);
        goto cleanup;
    }

    printf("Successfully started! \n");

    printf("%-8s %-7s %-2s %-20s > %-20s %s (%s)", "TIME", "PID", "IP",
           "SADDR:SPORT", "DADDR:DPORT", "STATE", "FLAGS");

    for (;;) {
        /* trigger our BPF program */
        err = perf_buffer__poll(pb, PERF_POLL_TIMEOUT_MS);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "error polling perf buffer: %s\n", strerror(-err));
            goto cleanup;
        }
        sleep(1);
    }

cleanup:
    perf_buffer__free(pb);
    test_tcptrace_bpf__destroy(skel);
    return -err;
}
