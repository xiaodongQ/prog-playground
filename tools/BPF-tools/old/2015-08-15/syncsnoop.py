#!/usr/bin/python
#
# syncsnoop.py	Trace sync() syscall.
#		For Linux, uses BCC, eBPF. Embedded C.
#
# Written as a basic example of BCC trace & reformat. See hello_world.py
# for a BCC trace with default output example.
#
# 13-Aug-2015	Brendan Gregg	Created this.

from bpf import BPF
import sys

# load BPF program
b = BPF(text = """
int do_sync(void *ctx) {
	bpf_trace_printk("sync()\\n");
	return 0;
};
""")
BPF.attach_kprobe(b.load_func("do_sync", BPF.KPROBE), "sys_sync")

# header
print "%-18s %s" % ("TIME(s)", "CALL")

# open trace pipe
try:
	trace = open("/sys/kernel/debug/tracing/trace_pipe", "r")
except:
	print >> sys.stderr, "ERROR: opening trace_pipe"
	exit(1)

# format output
while 1:
	try:
		line = trace.readline().rstrip()
	except KeyboardInterrupt:
		pass; exit()

	prolog, time_s, colon, word = line.rsplit(" ", 3)
	time_s = time_s[:-1]	# strip trailing ":"

	print "%-18s %s" % (time_s, word)
