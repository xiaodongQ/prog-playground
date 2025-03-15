#!/usr/bin/python
#
# vfscount.py	Count some VFS calls.
#		For Linux, uses BCC, eBPF. See .c file.
#
# Written as a basic example of counting functions.
#
# 14-Aug-2015	Brendan Gregg	Created this.

from bpf import BPF
from ctypes import c_ushort, c_int, c_ulonglong
from time import sleep, strftime
from sys import stderr

# kernel symbol translation
ksym_addrs = []			# addresses for binary search
ksym_names = []			# same index as ksym_addrs
def load_kallsyms():
	symfile = "/proc/kallsyms"
	try:
		syms = open(symfile, "r")
	except:
		print >> stderr, "ERROR: reading " + symfile
		exit()
	line = syms.readline()
	for line in iter(syms):
		cols = line.split()
		name = cols[2]
		if name[:4] != "vfs_":	# perf optimization
			continue
		addr = int(cols[0], 16)
		ksym_addrs.append(addr)
		ksym_names.append(name)
	syms.close()
def ksym(addr):
	start = -1
	end = len(ksym_addrs)
	while end != start + 1:
		mid = (start + end) / 2
		if addr < ksym_addrs[mid]:
			end = mid
		else:
			start = mid
	if start == -1:
		return "[unknown]"
	return ksym_names[start]
load_kallsyms()

# load BPF program
b = BPF(src_file = "vfscount.c")
fn = b.load_func("do_count", BPF.KPROBE)
BPF.attach_kprobe(fn, "vfs_read")
BPF.attach_kprobe(fn, "vfs_write")
BPF.attach_kprobe(fn, "vfs_fsync")
BPF.attach_kprobe(fn, "vfs_open")
BPF.attach_kprobe(fn, "vfs_create")
counts = b.get_table("counts")

# header
print "Tracing... Ctrl-C to end."

# output
try:
	sleep(99999999)
except KeyboardInterrupt:
	pass

print "\n%-16s %-12s %8s" % ("ADDR", "FUNC", "COUNT")
for k, v in sorted(counts.items(), key=lambda counts: counts[1].value):
	print "%-16x %-12s %8d" % (k.ip, ksym(k.ip), v.value)
