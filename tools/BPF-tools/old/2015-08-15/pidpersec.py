#!/usr/bin/python
#
# pidpersec.py	Count new processes (via fork).
#		For Linux, uses BCC, eBPF. See .c file.
#
# Written as a basic example of counting an event.
#
# 11-Aug-2015	Brendan Gregg	Created this.

from bpf import BPF
from ctypes import c_ushort, c_int, c_ulonglong
from time import sleep, strftime

# load BPF program
b = BPF(src_file = "pidpersec.c")
BPF.attach_kprobe(b.load_func("do_count", BPF.KPROBE), "sched_fork")
stats = b.get_table("stats", c_int, c_ulonglong)

# stat indexes
S_COUNT = 1

# header
print "Tracing... Ctrl-C to end."

# output
last = 0
while (1):
	try:
		sleep(1)
	except KeyboardInterrupt:
		pass; exit()

	print "%s: PIDs/sec: %d" % (strftime("%H:%M:%S"),
	    (stats[c_int(S_COUNT)].value - last))
	last = stats[c_int(S_COUNT)].value
