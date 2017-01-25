#!/bin/bash

if [ x$1 == x ]; then
   echo "Us: $0" '<particles input file>'
else
   args=(`awk -F- '{ print $1 " " $2 " "  $3 " " $4 " " $5 ; }' <<+++
$1
+++
`)

arg3=`sed s/.in// <<+++
${args[3]}
+++
`

./copy-nbody-cluster-2.sh

echo "Executing N-body on 2 boards..."

echo mpirun -np 2 -hostfile hosts nbody.sh 3 ${args[1]} $arg3  0
sleep 1
mpirun -np 2 -hostfile hosts nbody.sh 3 ${args[1]} $arg3  0

fi
