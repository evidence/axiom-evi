#!/bin/sh

echo 1 > /proc/sys/vm/overcommit_memory
echo 0 > /proc/sys/kernel/randomize_va_space

modprobe axiom-netdev && modprobe axiom_mem_dev && axiom-init $1 &
