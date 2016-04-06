#!/bin/bash

BUILDROOT=${BUILDROOT:-../axiom-evi-buildroot}
IMAGES=${IMAGES:-$BUILDROOT/output/images}
QEMU=${QEMU:-../axiom-evi-qemu/aarch64-softmmu/qemu-system-aarch64}
ID=${ID:-0}

SERIAL_VIDEO="-serial chardev:char1"
SERIAL_CONSOLE="-serial mon:stdio -display none"

CONSOLE=0

function usage
{
    echo -e "usage: ./start_qemu.sh [OPTION...]"
    echo -e ""
    echo -e " -q, --qemu      qemu_exe    use this qemu executable"
    echo -e " -d, --id        id          use this VM id"
    echo -e " -i, --images    img_dir     dir that contains kernel,initrd and dtb"
    echo -e " -c, --console               use stdio console"
    echo -e " -h, --help                  print this help"
}

while [ "$1" != "" ]; do
    case $1 in
        -q | --qemu )
            shift
            QEMU=$1
            ;;
        -d | --id )
            shift
            ID=$1
            ;;
        -c | --console )
            CONSOLE=1
            ;;
        -i | --images )
            shift
            IMAGES=$1
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

if [ "$CONSOLE" = "1" ]; then
    SERIAL=$SERIAL_CONSOLE
else
    SERIAL=$SERIAL_VIDEO
fi

set -x

${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256M -serial /dev/null ${SERIAL} -kernel ${IMAGES}/uImage -dtb ${IMAGES}/zynq-zc706.dtb --initrd ${IMAGES}/rootfs.cpio.uboot -net nic,vlan=0 -net user,hostfwd=tcp::2220${ID}-:22,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:3330${ID} -append "mem=256M" -chardev vc,id=char1
