#!/bin/bash

if [ x$1 == x ]; then
   echo "Us: $0" '<particles input file>'
else
   args=(`awk -F- '{ print $1 " " $2 " "  $3 " " $4 " " $5 ; }' <<+++
$1
+++
`)


echo "Executing N-body on 1 board..."

./nbody.sh 1 ${args[1]} ${args[3]}   0
./nbody.sh 2 ${args[1]} ${args[3]}   0
./nbody.sh 3 ${args[1]} ${args[3]}   0
./nbody.sh 4 ${args[1]} ${args[3]}   0

fi

