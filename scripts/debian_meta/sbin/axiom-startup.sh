#!/bin/bash

NODES=2

function usage () {
    echo -e "\nUsage: $0 [-n NODES]  NODEID\n"
    echo -e "Load all AXIOM modules and start axiom-init and axiom-ethtap."
    echo -e "NODEID and NODES are required, because for now we use static routing."
    echo -e "\nExample: $0 1\n"

    echo -e "\noptions (specified before NODEID):\n"
    echo -e "    -n  NODES   specify the number of nodes in the cluster [default $NODES]\n"
    echo -e "    -h          print this help\n"

}

while getopts ":n:h" opt; do
    case $opt in
        n)
            NODES=$OPTARG
            ;;
        h)
            usage
            exit 0
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            usage
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument." >&2
            usage
            exit 1
            ;;
    esac

done
shift $((OPTIND-1))

if [ x$1 == x ]; then
    usage
    exit 1
fi

ID=$1

# Make sure only root can run this script
if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

systemctl stop axiom-startup 
sleep 1

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

# generate the static routing table
RT_FILE="/tmp/axiom_static_rt.txt"

echo "-1" > $RT_FILE

for i in `seq 1 $NODES`; do
    if [ "$i" -eq "$ID" ]; then
        echo "0" >> $RT_FILE
    else
        echo "1" >> $RT_FILE
    fi
done

axiom-init -n ${ID} -r ${RT_FILE} &

sleep 1
axiom-ethtap -n 8 &
sleep 2

ifconfig ax0 192.168.17.${ID} mtu 65500

echo -e "\nNode ${ID} started!  IPoA: 192.168.17.${ID}\n"
