#!/bin/bash

function usage
{
	echo -e "usage: $0 [OPTION...]"
	echo -e ""
	echo -e " -n, --nodes      nodes     number of nodes"
	echo -e " -o, --outputdir  dir	     directory that contains the files for the guests"
	echo -e " -h, --help                 print this help"
}

NODES=2
OUTPUTDIR=./output

while [ "$1" != "" ]; do
	case $1 in
	-n | --nodes)
		shift
		NODES=$1
		;;
	-o | --outputdir)
		shift
		OUTPUTDIR=$1
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

set -x
for n in `seq 0 $((${NODES}-1))`;
do
	ssh-keygen -f "${HOME}/.ssh/known_hosts" -R [localhost]:2220$n
	sshpass -p root scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -P 2220$n -r ${OUTPUTDIR}/* root@localhost:
done
