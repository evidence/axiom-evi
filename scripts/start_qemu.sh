#!/bin/bash

BUILDROOT=${BUILDROOT:-../axiom-evi-buildroot}
IMAGES=${IMAGES:-../output/images}
QEMU=${QEMU:-../axiom-evi-qemu/build/aarch64-softmmu/qemu-system-aarch64}
ID=${ID:-0}
BOARD="zynqmp"
ARM_FIRMWARE="../arm-trusted-firmware/build/zynqmp/release/bl31"
QEMU_DTS="./buildroot/qemu-dts"
AXIOM_DTB="axiom-board.dtb"
USEPETALINUX=0
QEMU_DTB_FILE="${QEMU_DTS}/${AXIOM_DTB}"
S_MASTERROOTFS=${S_MASTERROOTFS:-${IMAGES}/zynq-rootfs_seco-snap-${ID}.qcow2}
B_MASTERROOTFS=${B_MASTERROOTFS:-${IMAGES}/zynq-rootfs_br-snap-${ID}.qcow2}
SERIAL_VIDEO="-serial chardev:char1"
SERIAL_CONSOLE="-serial mon:stdio -display none"

CONSOLE=0
LONG_ENQUEUE=0
NETWORK=${NETWORK:-0}

TAPNAME=tapax
MAC=1a:46:0b:ca:bc
SYSNET=/sys/devices/virtual/net
function if2dev {
    ls -d $SYSNET/$1/tap* | sed 's,.*tap,/dev/tap,'
}
function if2exist {
    [ -d $SYSNET/$1 ]
}

function usage
{
    echo -e "usage: ./start_qemu.sh [OPTION...]"
    echo -e ""
    echo -e " -b, --board     board       use this board [zynq|zynqmp]"
    echo -e " -q, --qemu      qemu_exe    use this qemu executable"
    echo -e " -d, --id        id          use this VM id"
    echo -e " -i, --images    img_dir     dir that contains kernel,initrd and dtb"
    echo -e " -c, --console               use stdio console"
    echo -e " -n, --network               create a tap network interface for guest comunication"
    echo -e " -l, --longenq               enqueue long message when there is no space left"
    echo -e " -p, --petalinux             use petalinux images (kernel, uboot, tdb)"
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
        -n | --network )
            NETWORK=1
            ;;
        -i | --images )
            shift
            IMAGES=$1
            ;;
        -l | --longenq )
            LONG_ENQUEUE=1
            ;;
        -p | --petalinux )
            USEPETALINUX=1
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

if [ "$LONG_ENQUEUE" = "1" ]; then
    AXIOM_DTB="axiom-board-long-enqueue.dtb"
fi

NETLINE="-net nic,vlan=1 -net none,vlan=1"
if  [ $NETWORK -eq 1 ]; then
    if2exist $TAPNAME$ID || { echo "interface $TAPNAME$ID does not exist! have \
you run 'make netup' on host?"; exit 255; }
    NETLINE="-net nic,macaddr=$MAC:$ID,vlan=3 -net tap,vlan=3,fd=42 	\
        42<>$(if2dev $TAPNAME$ID)"
fi

set -x

FIRMWARE_FILE=${ARM_FIRMWARE}/bl31.elf
KERNEL_FILE=${IMAGES}/Image
DTB_FILE=${IMAGES}/zynqmp-zcu102.dtb
#UBOOT_FILE=${IMAGES}/u-boot.elf
UBOOT_FILE=${IMAGES}/u-boot.sd.elf
SD_LINE="-drive file=${B_MASTERROOTFS},if=sd,index=1"

if [ X"$USEPETALINUX" = X"1" ]; then
#    FIRMWARE_FILE=${AXIOMBSP}/images/linux/bl31.elf
#    KERNEL_FILE=${AXIOMBSP}/images/linux/Image   
#    DTB_FILE=${AXIOMBSP}/images/linux/system.dtb
#    UBOOT_FILE=${AXIOMBSP}/images/linux/u-boot.elf
    UBOOT_FILE=${IMAGES}/u-boot.sd.elf
#    if [ ! -e ${KERNEL_FILE} ]; then
#	echo "AXIOMBSP environment variable not set or petalinux build not done"
#	exit 255
#    fi
    SD_LINE="-drive file=${S_MASTERROOTFS},if=sd,index=1"
#    QEMU="${PETALINUX}/tools/hsm/bin/qemu-system-aarch64"
#    QEMU_DTB_FILE="${PETALINUX}/tools/hsm/data/qemu/zynq/zed/system.dtb"
fi

# setenv bootargs root=/dev/mmcblk0 earlycon=cdns,mmio,0xff000000,115200n8 rw rootwait
# booti 0x00080000 - 0x06080000

if [ "$BOARD" = "zynq" ]; then
    eval ${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256       \
    -serial /dev/null ${SERIAL} -kernel ${IMAGES}/uImage                \
    -dtb ${IMAGES}/zynq-zc706.dtb --initrd ${IMAGES}/rootfs.cpio.uboot  \
    -net nic,vlan=0 -net user,hostfwd=tcp::2220${ID}-:22,vlan=0         \
    -net nic,vlan=1 -net none,vlan=1                                    \
    -net nic,vlan=2 -net socket,vlan=2,connect=localhost:3330${ID}      \
    -append "mem=256M" -chardev vc,id=char1 -redir tcp:1234{ID}::1234
elif [ "$BOARD" = "zynqmp" ]; then
    eval ${QEMU} -M arm-generic-fdt -m 1G ${SERIAL} -serial /dev/null \
    -rtc base=utc,clock=host -semihosting \
    -device loader,addr=0xfd1a0104,data=0x8000000e,data-len=4           \
    -device loader,file=${FIRMWARE_FILE},cpu=0                  \
    -device loader,file=${KERNEL_FILE},addr=0x00080000                 \
    -device loader,file=${DTB_FILE},addr=0x06080000     \
    -device loader,file=${UBOOT_FILE}                            \
    $SD_LINE \
    -hw-dtb ${QEMU_DTB_FILE}                                    \
    -net nic,vlan=1 -net none,vlan=1                                    \
    -net nic,vlan=1 -net none,vlan=1                                    \
    -net nic,vlan=0 -net user,hostfwd=tcp::2220${ID}-:22,vlan=0         \
    $NETLINE                                                            \
    -net nic,vlan=2 -net socket,vlan=2,connect=localhost:3330${ID}      \
    -chardev vc,id=char1 -redir tcp:1234${ID}::1234
else
    echo "Board $BOARD not supported"
    usage
fi

#    -device loader,file=${IMAGES}/zynqmp-zcu102.dtb,addr=0x04815c00    

#    -drive file=sdcard/axiom_sd.img,if=sd,index=1 \
