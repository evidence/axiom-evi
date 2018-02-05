#!/bin/bash

export LD_LIBRARY_PATH=/usr/lib/performance
#export LD_LIBRARY_PATH=/usr/lib/debug

set -x

NX_ARGS="--cluster --cluster-network=axiom --cluster-smp-presend=3" \
       NX_CLUSTER_NODE_MEMORY=67108864 \
       NX_GASNET_SEGMENT_SIZE=67108864 \
       axiom-run -P ompss $@
