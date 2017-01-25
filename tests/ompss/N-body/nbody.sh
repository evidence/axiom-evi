#!/bin/bash

# get number of threads as first argument and shift to the rest of arguments
NUM_TH=$1
shift

export NX_ARGS="--cluster --cluster-network mpi --smp-workers $NUM_TH --disable-binding --cluster-smp-presend 780 --cluster-node-memory $((1024*1024*(128))) --thd-output"

if [ $NUM_TH == 1 ]; then
   export NX_SCHEDULE=affinity-ready
else
   export NX_SCHEDULE=affinity
fi

echo "Using $NX_SCHEDULE scheduling policy"

#export NX_INSTRUMENTATION=extrae
export EXTRAE_CONFIG_FILE=./extrae.xml

export LD_BIND_NOW=1

echo "ompss/nbody $*"
ompss/nbody $*
#echo "ompss-deps/nbody $*"
#ompss-deps/nbody $*
#echo "serial/nbody $*"
#serial/nbody $*

exit 0
