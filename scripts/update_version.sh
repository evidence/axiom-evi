#!/bin/bash

PRINT=0
SET=0
VERSION=

function usage
{
    echo -e "usage: ./update_version.sh [OPTION...]"
    echo -e ""
    echo -e "update the version in all files"
    echo -e ""
    echo -e " -s, --set    vX.Y   set api version (vX.Y) in all files"
    echo -e " -p, --print         print api version in all files"
    echo -e " -h, --help          print this help"
}

while [ "$1" != "" ]; do
    case $1 in
        -s | --set )
            shift
            VERSION=$1
            SET=1
            ;;
        -p | --print )
            PRINT=1
            ;;
        -h | --help )
            usage
            exit
            ;;
        * )
            usage
            exit 1
    esac
    shift
done

VERSION_DOXY_RE=".*\* \\\version.*"
VERSION_DOXY_SUBS=" \* \\\version     ${VERSION}"

APPS_RE=$VERSION_DOXY_RE
APPS_SUBS=$VERSION_DOXY_SUBS
APPS_PATH="../axiom-evi-apps/*"
APPS_INCLUDE="--include *.c --include *.h"

NIC_RE=$VERSION_DOXY_RE
NIC_SUBS=$VERSION_DOXY_SUBS
NIC_PATH="../axiom-evi-nic/*"
NIC_INCLUDE="--include *.c --include *.h"

DRIVER_RE="MODULE_VERSION(.*"
DRIVER_SUBS="MODULE_VERSION(\"${VERSION}\");"
DRIVER_PATH="../axiom-evi-nic/axiom_netdev_driver/*"
DRIVER_INCLUDE="--include *.c"

DATASHEET_RE=".*\\\newcommand{\\\versionapi}.*"
DATASHEET_SUBS="\\\newcommand{\\\versionapi}{${VERSION}}"
DATASHEET_PATH="../axiom-evi-nic/axiom_docs/datasheet/*"
DATASHEET_INCLUDE="--include *.tex"

DOXYGEN_RE="PROJECT_NUMBER.*=.*"
DOXYGEN_SUBS="PROJECT_NUMBER         = ${VERSION}"
DOXYGEN_PATH="../axiom-evi-nic/axiom_docs/doxygen"
DOXYGEN_INCLUDE="--include *.doxyfile"

if [ "$SET" = "1" ]; then
    echo "setting version: $VERSION"
    set -x
    grep -rIl "$APPS_RE" $APPS_PATH $APPS_INCLUDE | \
        xargs sed -i -e "s/${APPS_RE}/${APPS_SUBS}/"
    grep -rIl "$DRIVER_RE" $DRIVER_PATH $DRIVER_INCLUDE | \
        xargs sed -i -e "s/${DRIVER_RE}/${DRIVER_SUBS}/"
    grep -rIl "$NIC_RE" $NIC_PATH $NIC_INCLUDE | \
        xargs sed -i -e "s/${NIC_RE}/${NIC_SUBS}/"
    grep -rIl "$DATASHEET_RE" $DATASHEET_PATH $DATASHEET_INCLUDE | \
        xargs sed -i -e "s/${DATASHEET_RE}/${DATASHEET_SUBS}/"
    grep -rIl "$DOXYGEN_RE" $DOXYGEN_PATH $DOXYGEN_INCLUDE | \
        xargs sed -i -e "s/${DOXYGEN_RE}/${DOXYGEN_SUBS}/"
elif [ "$PRINT" = "1" ]; then
    echo "printing version"
    grep -rnIe "$APPS_RE" $APPS_PATH $APPS_INCLUDE
    grep -rnIe "$DRIVER_RE" $DRIVER_PATH $DRIVER_INCLUDE
    grep -rnIe "$NIC_RE" $NIC_PATH $NIC_INCLUDE
    grep -rnIe "$DATASHEET_RE" $DATASHEET_PATH $DATASHEET_INCLUDE
    grep -rnIe "$DOXYGEN_RE" $DOXYGEN_PATH $DOXYGEN_INCLUDE
else
    usage
    exit 1
fi