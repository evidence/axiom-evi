#!/bin/bash

export LD_LIBRARY_PATH=/usr/lib/performance
#export LD_LIBRARY_PATH=/usr/lib/debug

set -x

NX_ARGS="--num-sockets=1 --cpus-per-socket=5 --smp-cpus=5 --use-my-cpus-number=1 --cluster --cluster-network=axiom --cluster-smp-presend=4" \
       NX_CLUSTER_NODE_MEMORY=67108864 \
       NX_GASNET_SEGMENT_SIZE=67108864 \
       AXIOM_WRK_PARAMS=OTHER,0 \
       AXIOM_COM_PARAMS=DEADLINE,5%,100m,100m \
       axiom-run -P ompss $@
