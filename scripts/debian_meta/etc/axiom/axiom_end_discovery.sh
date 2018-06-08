#!/bin/bash

# run by axiom-init at the end of discovery protocol

pkill axiom-ethtap 2>/dev/null 1>/dev/null
sleep 1

modprobe tun
axiom-ethtap -n 8 &

sleep 1

ID=`axiom-info -n -q`
ifconfig ax0 192.168.17.${ID} mtu 65500

echo "IPoverAXIOM set to subnet 192.168.17.0/24"
echo "Other nodes are reachable with IP: 192.168.17.NODEID"
echo "Local IP is 192.168.17.${ID}"

# setup NAT on master node and gateway and DNS on slave nodes
modprobe iptable_nat
if [ "$?" == "0" ]; then
    # master node
    if [ "${ID}" == "1" ]; then
        sysctl -w net.ipv4.ip_forward=1
        iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
        iptables -A FORWARD -i eth0 -o ax0 -m state --state RELATED,ESTABLISHED -j ACCEPT
        iptables -A FORWARD -i ax0 -o eth0 -j ACCEPT
    else #slave node
        # set default gateway
        ip route add default via 192.168.17.1
        # set google DNS
        echo -e "nameserver 8.8.8.8\nnameserver 8.8.4.4\n" > /etc/resolvconf/resolv.conf.d/tail
        resolvconf -u
    fi
    echo "NAT enabled. Internet reachable through Master node ethernet port"
fi
