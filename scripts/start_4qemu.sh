#!/bin/bash



export BUILDROOT=${BUILDROOT:-../axiom-buildroot}
export QEMU=${QEMU:-$HOME/usr/axiom/bin/qemu-system-aarch64}

#export QEMU=${QEMU:-/usr/bin/qemu-system-x86_64}
#export KERNEL==${KERNEL:-$BUILDROOT/output/images/zImage}
#export DRIVE==${DRIVE:-$BUILDROOT/output/images/rootfs.ext2}

set -x

#${QEMU} -M vexpress-a9 -m 512M -kernel $BUILDROOT/output/images/zImage -dtb $BUILDROOT/output/images/vexpress-v2p-ca9.dtb -drive file=$BUILDROOT/output/images/rootfs.ext2,if=sd,format=raw -append "console=ttyAMA0,115200 root=/dev/mmcblk0 mem=256M" -serial mon:stdio  -nographic -net nic,model=lan9118 -net user,hostfwd=tcp::20001-:22 -monitor tcp::10001,server,nowait,nodelay -device axiom-dev

#${QEMU} -M arm-generic-fdt-plnx -machine linux=on -serial /dev/null -serial mon:stdio -display none -kernel $BUILDROOT/output/images/uImage -dtb $BUILDROOT/output/images/zynq-zc706.dtb --initrd $BUILDROOT/output/images/rootfs.cpio.uboot -net nic,vlan=0 -net user,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net tap,ifname=tap0,vlan=2 -append "mem=512M" -redir tcp:1234::1234

${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256M -serial /dev/null -serial chardev:char1 -kernel $BUILDROOT/output/images/uImage -dtb $BUILDROOT/output/images/zynq-zc706.dtb --initrd $BUILDROOT/output/images/rootfs.cpio.uboot -net nic,vlan=0 -net user,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:33300 -append "mem=256M" -chardev vc,id=char1 &

${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256M -serial /dev/null -serial chardev:char1 -kernel $BUILDROOT/output/images/uImage -dtb $BUILDROOT/output/images/zynq-zc706.dtb --initrd $BUILDROOT/output/images/rootfs.cpio.uboot -net nic,vlan=0 -net user,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:33301 -append "mem=256M" -chardev vc,id=char1 &

${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256M -serial /dev/null -serial chardev:char1 -kernel $BUILDROOT/output/images/uImage -dtb $BUILDROOT/output/images/zynq-zc706.dtb --initrd $BUILDROOT/output/images/rootfs.cpio.uboot -net nic,vlan=0 -net user,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:33302 -append "mem=256M" -chardev vc,id=char1 &

${QEMU} -M arm-generic-fdt-plnx -machine linux=on -m 256M -serial /dev/null -serial chardev:char1 -kernel $BUILDROOT/output/images/uImage -dtb $BUILDROOT/output/images/zynq-zc706.dtb --initrd $BUILDROOT/output/images/rootfs.cpio.uboot -net nic,vlan=0 -net user,vlan=0 -net nic,vlan=1 -net none,vlan=1 -net nic,vlan=2 -net socket,vlan=2,connect=localhost:33303 -append "mem=256M" -chardev vc,id=char1 &
