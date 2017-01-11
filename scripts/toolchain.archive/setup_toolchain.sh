#!/bin/bash

function usage
{
	echo -e "usage: $0 [OPTION...]"
	echo -e ""
	echo -e " -t, --toolchain  dir	     directory that contains the toolchains [def: ./toolchain]"
	echo -e " -h, --help                 print this help"
}

MAGIC=__AXIOM_TOOLCHAIN__
TOOLCHAIN_DIR="./toolchain"

while [ "$1" != "" ]; do
	case $1 in
	-t | --toolchain)
		shift
		TOOLCHAIN_DIR=$1
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

TOOLCHAIN_DIR=$(realpath "$TOOLCHAIN_DIR")

set -x
sed -i "s,${MAGIC},${TOOLCHAIN_DIR}/host,g" "${TOOLCHAIN_DIR}"/host/usr/share/mcxx/config.d/*
