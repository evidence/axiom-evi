#!/bin/sh -e

NNODES=${VMS:-2}
NODES=$(seq 0 $((NNODES-1)))

PASSWD=root

# mac address is $MAC:$N
MAC=1a:46:0b:ca:bc

for N in $NODES; do
    ssh-keygen -f "$HOME/.ssh/known_hosts" -R [127.0.0.1]:2220$N

    sshpass -p $PASSWD rsync \
	    -av \
	    -e "ssh -o StrictHostKeyChecking=no -p 2220$N -l root" \
	    guest/init_network.sh 127.0.0.1:
    
    sshpass -p $PASSWD ssh \
	    -o StrictHostKeyChecking=no \
	    -p 2220$N \
	    root@127.0.0.1 \
            "/root/init_network.sh -n $NNODES $(test $N -eq 0 && echo '-m')"
    
done

exit 0
