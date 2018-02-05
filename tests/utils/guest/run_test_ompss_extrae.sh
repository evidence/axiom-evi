#!/bin/bash

export LD_LIBRARY_PATH=/usr/lib/instrumentation
export LD_PRELOAD=/usr/lib/libnanostrace.so

set -x

rm -rf set-0

NX_ARGS="--cluster --cluster-network=axiom --cluster-smp-presend=3 --instrumentation=extrae --instrument-default=developer" \
       NX_CLUSTER_NODE_MEMORY=67108864 \
       NX_GASNET_SEGMENT_SIZE=67108864 \
       NX_EXTRAE_AVOID_MERGE=1 \
       EXTRAE_CONFIG_FILE=extrae.xml \
       EXTRAE_HOME=/usr \
       axiom-run -P ompss $@
