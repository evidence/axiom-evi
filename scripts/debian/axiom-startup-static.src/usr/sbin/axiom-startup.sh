#!/bin/bash

if [ x$1 == x ]; then
    echo -e "\nUsage: $0 NODEID\n"
    echo -e "Load all AXIOM modules and start axiom-init and axiom-ethtap."
    echo -e "NODEID is required, because for now we use static routing."
    echo -e "\nExample: $0 1\n"
    exit 1
fi

# Make sure only root can run this script
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

ID=$1

pkill axiom-ethtap > /dev/null 2>&1
pkill axiom-init > /dev/null 2>&1
sleep 1
modprobe -r axiom-netdev > /dev/null 2>&1
modprobe -r tun > /dev/null 2>&1

set -x

echo 1 > /proc/sys/vm/overcommit_memory
echo 0 > /proc/sys/kernel/randomize_va_space

modprobe axiom-netdev
modprobe tun

axiom-init -n $1 -r /etc/axiom/axiom_static_rt_node_${ID}.txt &

sleep 1
axiom-ethtap -n 8 &
sleep 2

ifconfig ax0 192.168.17.${ID} mtu 65500

echo -e "\nNode ${ID} started!  IPoA: 192.168.17.${ID}\n"
