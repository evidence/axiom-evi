#!/bin/sh -e

# dummy ethernet card is $BRIF
BRIF=axiombr
# tap name is $TAPNAME$ID
TAPNAME=tapax
# mac address is $MAC:$ID
MAC=1a:46:0b:ca:bc
# ip address is "echo $IP | sed 's/XXX/$ID/'"
IP=172.16.XXX.1

NNODES=4

[ $(whoami) != 'root' ] && echo "You must be root!" >/dev/stderr && exit 255

UP=1
case $1 in
    up)
	UP=1
    ;;
    down)
	UP=0
    ;;
    *)
	echo "usage: network.sh [up|down]" >/dev/stderr
	exit 255
    ;;
esac

SYSNET=/sys/devices/virtual/net

if2dev() {
    ls -d $SYSNET/$1/tap*|sed 's,.*tap,/dev/tap,'
}

if [ $UP -eq 1 ]; then

    #
    # create network
    #
    
    ip link add $BRIF type dummy
    ip link set $BRIF up
    
    for ID in $(seq 0 $((NNODES-1))); do
	ip link add link $BRIF address $MAC:$ID name $TAPNAME$ID type macvtap mode bridge
	ip link set $TAPNAME$ID up
	# WARN: it seems that the permissions are not set without a delay!!!!
	sleep 0.15
	chmod og+rw $(if2dev $TAPNAME$ID)
    done

    ip link add link $BRIF address $MAC:$NNODES name $TAPNAME$NNODES type macvlan mode bridge
    ip link set $TAPNAME$NNODES up
    ifconfig $TAPNAME$NNODES $(echo $IP | sed "s/XXX/255/")

else

    #
    # destroy network
    #
    
    for ID in $(seq 0 $((NNODES-1))); do
	ip link delete dev $TAPNAME$ID type macvtap || true
    done
    ip link delete dev $TAPNAME$NNODES type macvlan || true
	
    ip link delete dev $BRIF type dummy || true

fi
