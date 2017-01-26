#!/bin/sh

if [ x$1 == x ]; then
   echo "Us: $0" '<particles input file>'
else

ARG1=`echo $1 | sed -e 's/[a-z]*-//' -e 's/-.*//'`
ARG3=`echo $1 |sed -e 's/.*-//' -e 's/\..*//'`
    
echo "Executing N-body on `axiom-info -N -q` boards..."

NUM_TH=3

#
#
#
rm -rf set-0

export NX_ARGS="--cluster --cluster-network axiom --instrumentation extrae --smp-workers $NUM_TH --disable-binding --cluster-smp-presend 780 --cluster-node-memory $((1024*1024*(128))) --thd-output"

if [ $NUM_TH == 1 ]; then
   export NX_SCHEDULE=affinity-ready
else
   export NX_SCHEDULE=affinity
fi
echo "Using $NX_SCHEDULE scheduling policy"

export LD_BIND_NOW=1
export LD_LIBRARY_PATH=/usr/lib/instrumentation
export LD_PRELOAD=/usr/lib/libnanostrace.so

export EXTRAE_CONFIG_FILE=./extrae.xml
export NX_EXTRAE_AVOID_MERGE=1
export EXTRAE_HOME=/usr
   
echo axiom-run -P ompss ./nbody $ARG1 $ARG3 0
axiom-run -P ompss ./nbody $ARG1 $ARG3 0

fi
