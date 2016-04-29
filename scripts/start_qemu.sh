#!/bin/bash

BUILDROOT=${BUILDROOT:-../axiom-evi-buildroot}
IMAGES=${IMAGES:-$BUILDROOT/output/images}
QEMU=${QEMU:-../axiom-evi-qemu/aarch64-softmmu/qemu-system-aarch64}
ID=${ID:-0}
BOARD="zynqmp"
ARM_FIRMWARE="../arm-trusted-firmware"
QEMU_DTS="./buildroot/qemu-dts"

SERIAL_VIDEO="-serial chardev:char1"
SERIAL_CONSOLE="-serial mon:stdio -display none"

CONSOLE=0

function usage
{
    echo -e "usage: ./start_qemu.sh [OPTION...]"
    echo -e ""
    echo -e " -b, --board     board       use this board [zynq|zynqmp]"
    echo -e " -q, --qemu      qemu_exe    use this qemu executable"
    echo -e " -d, --id        id          use this VM id"
    echo -e " -i, --images    img_dir     dir that contains kernel,initrd and dtb"
    echo -e " -c, --console               use stdio console"
    echo -e " -h, --help                  print this help"
}

while [ "$1" != "" ]; do
    case $1 in
        -b | --board )
            shift
            BOARD=$1
            ;;
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

if [ "$BOARD" = "zynq" ]; then
    ${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256M -serial /dev/null ${SERIAL} -kernel ${IMAGES}/uImage -dtb ${IMAGES}/zynq-zc706.dtb --initrd ${IMAGES}/rootfs.cpio.uboot -net nic,vlan=0 -net user,hostfwd=tcp::2220${ID}-:22,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:3330${ID} -append "mem=256M" -chardev vc,id=char1
elif [ "$BOARD" = "zynqmp" ]; then
    ${QEMU} -M arm-generic-fdt -m 256M ${SERIAL} -serial /dev/null -device loader,addr=0xfd1a0104,data=0x8000000e,data-len=4 -device loader,file=${ARM_FIRMWARE}/build/zynqmp/release/bl31/bl31.elf,cpu=0 -device loader,file=${IMAGES}/Image,addr=0x00080000 -device loader,file=${IMAGES}/zynqmp-zcu102.dtb,addr=0x04080000 -device loader,file=${IMAGES}/u-boot.elf -hw-dtb ${QEMU_DTS}/zynqmp-qemu-arm.dtb -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=0 -net user,hostfwd=tcp::2220${ID}-:22,vlan=0 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:3330${ID} -chardev vc,id=char1
else
    echo "Board $BOARD not supported"
    usage
fi
