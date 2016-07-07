#!/bin/sh -e

PATH=/root:/bin:/sbin:/usr/bin:/usr/sbin

#default number of nodes
NNODES=2
# arguments for axiom-init
ARGS=""
# 1 if I am the master node
MASTER=0

CONT=1
while [ $CONT -eq 1 ]; do
    case $1 in
	-n)
	    NNODES=$2
	    shift 2	    
	    ;;
	-m)
	    ARGS="-m"
	    MASTER=1
	    shift
	    ;;
	-a)
	    MYMAC=$2
	    shift 2
	    ;;
	*)
	    CONT=0;
	    ;;
    esac
done

# ip address is "echo $IP | sed 's/XXX/$ID/'"
IP=172.16.XXX.1/16
# mac address is $MAC:$ID
MAC=1a:46:0b:ca:bc

killall axiom-init || true
rmmod axiom_netdev || true
modprobe axiom_netdev 
axiom-init "$ARGS" &

# if I am not the master wait my node id....
if [ $MASTER -eq 0 ]; then
    while [ $(axiom-info -q -n) -eq 0 ]; do
	sleep 0.15
    done    
fi

# my node id
ID=$(axiom-info -q -n)
# compute my node IP
MYIP=$(echo $IP | sed "s/XXX/$ID/")
# compute my node mac if not set
[ -z "$MYMAC" ] && MYMAC=$MAC:$ID

# MYMAC=$(ip link show eth1 | grep "link/ether" | awk '{print $2}')

echo MY IP is $MYIP
echo MY MAC is $MYMAC

ip link set dev eth1 down
ip link set dev eth1 address $MYMAC 
for IP in $(ip addr show eth1 | grep "inet " | awk '{print $2}'); do
    ip addr del $IP dev eth1
done
ip addr add $MYIP dev eth1
ip link set dev eth1 up
