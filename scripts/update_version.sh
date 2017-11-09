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
    echo -e " -s, --set    X.Y   set api version (vX.Y) in all files"
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
VERSION_DOXY_SUBS=" \* \\\version     v${VERSION}"

APPS_RE=$VERSION_DOXY_RE
APPS_SUBS=$VERSION_DOXY_SUBS
APPS_PATH="../axiom-evi-apps"
APPS_INCLUDE="--include *.c --include *.h"

NIC_RE=$VERSION_DOXY_RE
NIC_SUBS=$VERSION_DOXY_SUBS
NIC_PATH="../axiom-evi-nic"
NIC_INCLUDE="--include *.c --include *.h"

ALLOC_RE=$VERSION_DOXY_RE
ALLOC_SUBS=$VERSION_DOXY_SUBS
ALLOC_PATH="../axiom-allocator"
ALLOC_INCLUDE="--include *.c --include *.h"

TESTS_RE=$VERSION_DOXY_RE
TESTS_SUBS=$VERSION_DOXY_SUBS
TESTS_PATH="../tests"
TESTS_INCLUDE="--include *.c --include *.h"

MEMLIB_RE=$VERSION_DOXY_RE
MEMLIB_SUBS=$VERSION_DOXY_SUBS
MEMLIB_PATH="../axiom-evi-allocator-lib"
MEMLIB_INCLUDE="--include *.c --include *.h"

MEMDRV_RE=$VERSION_DOXY_RE
MEMDRV_SUBS=$VERSION_DOXY_SUBS
MEMDRV_PATH="../axiom-evi-allocator-drv"
MEMDRV_INCLUDE="--include *.c --include *.h"

MEMDRV2_RE="MODULE_VERSION(.*"
MEMDRV2_SUBS="MODULE_VERSION(\"v${VERSION}\");"
MEMDRV2_PATH="../axiom-evi-allocator-drv"
MEMDRV2_INCLUDE="--include *.c"

DRIVER_RE="MODULE_VERSION(.*"
DRIVER_SUBS="MODULE_VERSION(\"v${VERSION}\");"
DRIVER_PATH="../axiom-evi-nic/axiom_netdev_driver"
DRIVER_INCLUDE="--include *.c"

API_RE="#define AXIOM_API_VERSION_STR.*"
API_SUBS="#define AXIOM_API_VERSION_STR \"v${VERSION}\""
API_PATH="../axiom-evi-nic/include"
API_INCLUDE="--include *.h"

DATASHEET_RE=".*\\\newcommand{\\\versionapi}.*"
DATASHEET_SUBS="\\\newcommand{\\\versionapi}{v${VERSION}}"
DATASHEET_PATH="../axiom-evi-nic/axiom_docs/datasheet"
DATASHEET_INCLUDE="--include *.tex"

DOXYGEN_RE="PROJECT_NUMBER.*=.*"
DOXYGEN_SUBS="PROJECT_NUMBER         = v${VERSION}"
DOXYGEN_PATH="../axiom-evi-nic/axiom_docs/doxygen"
DOXYGEN_INCLUDE="--include *.doxyfile"

GASNET_RE="#define GASNET_CORE_VERSION .*"
GASNET_SUBS="#define GASNET_CORE_VERSION      ${VERSION}"
GASNET_PATH="../axiom-evi-gasnet/axiom-conduit"
GASNET_INCLUDE="--include *.h"

if [ "$SET" = "1" ]; then
    echo "setting version: $VERSION"
    set -x
    grep -rIl "$APPS_RE" "$APPS_PATH" $APPS_INCLUDE | \
        xargs sed -i -e "s/${APPS_RE}/${APPS_SUBS}/"
    grep -rIl "$DRIVER_RE" "$DRIVER_PATH" $DRIVER_INCLUDE | \
        xargs sed -i -e "s/${DRIVER_RE}/${DRIVER_SUBS}/"
    grep -rIl "$NIC_RE" "$NIC_PATH" $NIC_INCLUDE | \
        xargs sed -i -e "s/${NIC_RE}/${NIC_SUBS}/"
    grep -rIl "$API_RE" "$API_PATH" $API_INCLUDE | \
        xargs sed -i -e "s/${API_RE}/${API_SUBS}/"
    grep -rIl "$ALLOC_RE" "$ALLOC_PATH" $ALLOC_INCLUDE | \
        xargs sed -i -e "s/${ALLOC_RE}/${ALLOC_SUBS}/"
    grep -rIl "$TESTS_RE" "$TESTS_PATH" $TESTS_INCLUDE | \
        xargs sed -i -e "s/${TESTS_RE}/${TESTS_SUBS}/"
    grep -rIl "$MEMLIB_RE" "$MEMLIB_PATH" $MEMLIB_INCLUDE | \
        xargs sed -i -e "s/${MEMLIB_RE}/${MEMLIB_SUBS}/"
    grep -rIl "$MEMDRV_RE" "$MEMDRV_PATH" $MEMDRV_INCLUDE | \
        xargs sed -i -e "s/${MEMDRV_RE}/${MEMDRV_SUBS}/"
    grep -rIl "$MEMDRV2_RE" "$MEMDRV2_PATH" $MEMDRV2_INCLUDE | \
        xargs sed -i -e "s/${MEMDRV2_RE}/${MEMDRV2_SUBS}/"
    grep -rIl "$DATASHEET_RE" "$DATASHEET_PATH" $DATASHEET_INCLUDE | \
        xargs sed -i -e "s/${DATASHEET_RE}/${DATASHEET_SUBS}/"
    grep -rIl "$DOXYGEN_RE" "$DOXYGEN_PATH" $DOXYGEN_INCLUDE | \
        xargs sed -i -e "s/${DOXYGEN_RE}/${DOXYGEN_SUBS}/"
    grep -rIl "$GASNET_RE" "$GASNET_PATH" $GASNET_INCLUDE | \
        xargs sed -i -e "s/${GASNET_RE}/${GASNET_SUBS}/"
elif [ "$PRINT" = "1" ]; then
    echo "printing version"
    grep -rnIe "$APPS_RE" "$APPS_PATH" $APPS_INCLUDE
    grep -rnIe "$DRIVER_RE" "$DRIVER_PATH" $DRIVER_INCLUDE
    grep -rnIe "$NIC_RE" "$NIC_PATH" $NIC_INCLUDE
    grep -rnIe "$API_RE" "$API_PATH" $API_INCLUDE
    grep -rnIe "$ALLOC_RE" "$ALLOC_PATH" $ALLOC_INCLUDE
    grep -rnIe "$TESTS_RE" "$TESTS_PATH" $TESTS_INCLUDE
    grep -rnIe "$MEMLIB_RE" "$MEMLIB_PATH" $MEMLIB_INCLUDE
    grep -rnIe "$MEMDRV_RE" "$MEMDRV_PATH" $MEMDRV_INCLUDE
    grep -rnIe "$MEMDRV2_RE" "$MEMDRV2_PATH" $MEMDRV2_INCLUDE
    grep -rnIe "$DATASHEET_RE" "$DATASHEET_PATH" $DATASHEET_INCLUDE
    grep -rnIe "$DOXYGEN_RE" "$DOXYGEN_PATH" $DOXYGEN_INCLUDE
    grep -rnIe "$GASNET_RE" "$GASNET_PATH" $GASNET_INCLUDE
else
    usage
    exit 1
fi
