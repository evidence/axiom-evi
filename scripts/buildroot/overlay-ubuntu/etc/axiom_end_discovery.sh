#!/bin/sh

# run by axiom-init at the end of discovery protocol

[ -e /dev/net ] || mkdir /dev/net
[ -e /dev/net/tun ] || mknod /dev/net/tun c 10 200

killall axiom-ethtap 2>/dev/null 1>/dev/null
modprobe tun
axiom-ethtap &

sleep 2

ID=`axiom-info -n -q`
ifconfig ax0 192.168.17.${ID}

echo "My IP is 192.168.17.${ID}"
