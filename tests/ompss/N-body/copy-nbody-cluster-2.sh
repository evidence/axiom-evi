#!/bin/bash
scp -p nbody.sh udooer@udoo2:N-body
#scp -p nbody.sh udooer@udoo1:N-body
scp -p extrae.xml udooer@udoo2:N-body
#scp -p extrae.xml udooer@udoo1:N-body
scp -p ompss/nbody udooer@udoo2:N-body/ompss
#scp -p ompss/nbody udooer@udoo1:N-body/ompss
scp -p ompss-deps/nbody udooer@udoo2:N-body/ompss-deps
#scp -p ompss-deps/nbody udooer@udoo1:N-body/ompss-deps

