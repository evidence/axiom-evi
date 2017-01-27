#!/bin/sh

if [ x$1 == x ]; then
    echo "Use: $0 NUM_PARTICLES NUM_STEPS"
    echo "Example: $0 256 5"
    echo "(for using file 'particles-256-128-5.in')"
else
    NUM_PART=$1
    NUM_STEP=$2
    NUM_THREADS=1

    echo "Using file 'particles-${NUM_PART}-128-${NUM_STEP}.in'"        
    echo "Executing N-body on `axiom-info -N -q` boards..."

    #export NX_ARGS="--cluster --cluster-network axiom --instrumentation extrae --smp-workers $NUM_THREADS --disable-binding --cluster-smp-presend 780 --thd-output"
    export NX_ARGS="--cluster --cluster-network axiom --instrumentation extrae --smp-workers $NUM_THREADS --disable-binding --thd-output"
    export NX_GASNET_SEGMENT_SIZE=268435456
    export NX_CLUSTER_NODE_MEMORY=134217728
    if [ $NUM_THREADS == 1 ]; then
    	export NX_SCHEDULE=affinity-ready
    else
    	export NX_SCHEDULE=affinity
    fi

    export LD_BIND_NOW=1
    export LD_LIBRARY_PATH=/usr/lib/instrumentation
    export LD_PRELOAD=/usr/lib/libnanostrace.so
    
    export EXTRAE_CONFIG_FILE=./extrae.xml
    export NX_EXTRAE_AVOID_MERGE=1
    export EXTRAE_HOME=/usr
    rm -rf set-0
    
    echo axiom-run -P ompss ./nbody $NUM_PART $NUM_STEP 0
    axiom-run -P ompss ./nbody $NUM_PART $NUM_STEP 0
fi
