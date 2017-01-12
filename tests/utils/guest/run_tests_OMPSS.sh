#!/bin/sh

export LD_LIBRARY_PATH=/usr/lib/performance

set -x

NX_ARGS="--cluster --cluster-network=axiom --smp-workers=1" \
       NX_CLUSTER_NODE_MEMORY=16777216 \
       NX_GASNET_SEGMENT_SIZE=67108864 \
       axiom-run -P ompss $@
