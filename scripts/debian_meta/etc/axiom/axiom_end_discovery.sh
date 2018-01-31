#!/bin/sh

# run by axiom-init at the end of discovery protocol

pkill axiom-ethtap 2>/dev/null 1>/dev/null
sleep 1

modprobe tun
axiom-ethtap -n 8 &

sleep 1

ID=`axiom-info -n -q`
ifconfig ax0 192.168.17.${ID} mtu 65500

echo "My IP is 192.168.17.${ID}"
