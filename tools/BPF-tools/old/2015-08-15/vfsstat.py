#!/usr/bin/python
#
# vfsstat.py	Count some VFS calls.
#		For Linux, uses BCC, eBPF. See .c file.
#
# Written as a basic example of counting multiple events as a stat tool.
#
# USAGE: vfsstat.py [interval [count]]
#
# 14-Aug-2015	Brendan Gregg	Created this.

from __future__ import print_function
from bpf import BPF
from ctypes import c_ushort, c_int, c_ulonglong
from time import sleep, strftime
from sys import argv

def usage():
	print("USAGE: %s [interval [count]]" % argv[0])
	exit()

# arguments
interval = 1
count = -1
if len(argv) > 1:
	try:
		interval = int(argv[1])
		if interval == 0:
			raise
		if len(argv) > 2:
			count = int(argv[2])
	except:	# also catches -h, --help
		usage()

# load BPF program
b = BPF(src_file = "vfsstat.c")
BPF.attach_kprobe(b.load_func("do_read", BPF.KPROBE), "vfs_read")
BPF.attach_kprobe(b.load_func("do_write", BPF.KPROBE), "vfs_write")
BPF.attach_kprobe(b.load_func("do_fsync", BPF.KPROBE), "vfs_fsync")
BPF.attach_kprobe(b.load_func("do_open", BPF.KPROBE), "vfs_open")
BPF.attach_kprobe(b.load_func("do_create", BPF.KPROBE), "vfs_create")
stats = b.get_table("stats", c_int, c_ulonglong)

# stat column labels and indexes
stat_types = {
	"READ" : 1,
	"WRITE" : 2,
	"FSYNC" : 3,
	"OPEN" : 4,
	"CREATE" : 5
}

# header
print("%-8s  " % "TIME", end="")
last = {}
for stype in stat_types.keys():
	print(" %8s" % (stype + "/s"), end="")
	idx = stat_types[stype]
	last[idx] = 0
print("")

# output
i = 0
while (1):
	if count > 0:
		i += 1
		if i > count:
			exit()
	try:
		sleep(interval)
	except KeyboardInterrupt:
		pass; exit()

	print("%-8s: " % strftime("%H:%M:%S"), end="")
	# print each statistic as a column
	for stype in stat_types.keys():
		idx = stat_types[stype]
		try:
			delta = stats[c_int(idx)].value - last[idx]
			print(" %8d" % (delta / interval), end="")
			last[idx] = stats[c_int(idx)].value
		except:
			print(" %8d" % 0, end="")
	print("")
