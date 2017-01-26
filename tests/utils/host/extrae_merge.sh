#!/bin/bash

function usage
{
	echo -e "usage: $0 [OPTION...] "
	echo -e ""
	echo -e " -n, --nodes      nodes     number of nodes"
	echo -e " -d, --dir        dir       guest trace directory"
	echo -e " -h, --help                 print this help"
}

DATE=$(date +%Y%m%d_%H%M%S)
NODES=2
TRACE_DIR=/root/tests_ompss/

OUTPUT_DIR=${DATE}_trace

while [ "$1" != "" ]; do
	case $1 in
	-n | --nodes)
		shift
		NODES=$1
		;;
	-d | --dir)
		shift
		TRACE_DIR=$1
		;;
	-h | --help)
		usage
		exit
		;;
	*)
		usage
		exit 1
	esac
	shift
done

TRACE_FILE=${TRACE_DIR}/TRACE.mpits

set -x
mkdir -p ${OUTPUT_DIR}/set-0
for n in `seq 0 $((${NODES}-1))`;
do
	ssh-keygen -f "${HOME}/.ssh/known_hosts" -R [localhost]:2220$n
	sshpass -p root scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -P 2220$n -r root@localhost:${TRACE_DIR}/set-0/* ${OUTPUT_DIR}/set-0/
	sshpass -p root scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -P 2220$n -r root@localhost:${TRACE_FILE} ${OUTPUT_DIR}/TRACE.$n.par
	cat ${OUTPUT_DIR}/TRACE.$n.par >> ${OUTPUT_DIR}/TRACE.mpits
done

./mpi2prv -f ${OUTPUT_DIR}/TRACE.mpits -o ${OUTPUT_DIR}/output.prv
