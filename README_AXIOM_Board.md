# AXIOM Software Stack compilation for the AXIOM Board (Xilinx UltraScale+)

----

## 1. Prerequisite to build the Software Stack

**NOTE: Some files listed in this README are not public available (eg. AXIOM
filesystem), please ask us for more details.**

### 1.1 AXIOM filesystem

You must have a copy of the AXIOM filesystem.
```
$ wget -c https://upload.axiom-project.eu/uploads/WP5.1/filesystems/aarch64/zynq-ubuntu-minimal_16.04-xenial_v1.3.tar.gz
$ mkdir zynq-ubuntu-minimal_16.04-xenial_v1.3
$ ln -s zynq-ubuntu-minimal_16.04-xenial_v1.3 zynq-rootfs
$ cd zynq-ubuntu-minimal_16.04-xenial_v1.3
$ tar xvf ../zynq-ubuntu-minimal_16.04-xenial_v1.3.tar.gz
```

### 1.2  Linaro

You must have the Linaro toolchain for AARCH64.
The directory of the Linaro toolchain must have a specific structure and must have
some link as show below.
You should use these linaro packages:
  * gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu.tar.xz
  * runtime-gcc-linaro-5.3.1-2016.05-aarch64-linux-gnu.tar.xz
  * sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu.tar.xz

```
$ mkdir linaro
$ cd linaro
$ wget -c  https://releases.linaro.org/components/toolchain/binaries/5.3-2016.05/aarch64-linux-gnu/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu.tar.xz
$ wget -c  https://releases.linaro.org/components/toolchain/binaries/5.3-2016.05/aarch64-linux-gnu/runtime-gcc-linaro-5.3.1-2016.05-aarch64-linux-gnu.tar.xz
$ wget -c  https://releases.linaro.org/components/toolchain/binaries/5.3-2016.05/aarch64-linux-gnu/sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu.tar.xz
$ tar xJvf gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu.tar.xz
$ tar xJvf runtime-gcc-linaro-5.3.1-2016.05-aarch64-linux-gnu.tar.xz
$ tar xJvf sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu.tar.xz
$ ln -s gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu host
$ ln -s sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu sysroot
$ ln -s . host/usr
$ ln -s /usr/bin/pkg-config host/bin/aarch64-linux-gnu-pkg-config
$ cd ..
```

### 1.3 Petalinux

You should have installed PetaLinux v2016.3
```
$ wget -c https://upload.axiom-project.eu/uploads/WP5.1/tools/petalinux-v2016.3-final-installer.run
$ ./petalinux-v2016.3-final-installer.run
$ ln -s petalinux-v2016.3-final petalinux

```

### 1.4 AXIOM BSP

You must have installed the last AXIOM BSP.
```
$ mkdir bsp
$ cd bsp
$ wget -c https://upload.axiom-project.eu/uploads/WP5.1/BSP_pre-built/AXIOM-ZU9EG-BSP-2016.3_v1.6.bsp
$ mv AXIOM-ZU9EG-BSP-2016.3_v1.6.bsp AXIOM-ZU9EG-BSP-2016.3_v1.6.bsp.tar.gz
$ tar xzvf  AXIOM-ZU9EG-BSP-2016.3_v1.6.bsp.tar.gz
$ mv AXIOM-ZU9EG-2016.3 AXIOM-ZU9EG-BSP-2016.3_v1.6
$ ln -s AXIOM-ZU9EG-BSP-2016.3_v1.6 AXIOM-ZU9EG-2016.3
$ cd ..
```

### 1.5 Compile the kernel using PetasLlinux

You must compile the AXIOM linux kernel with PetaLinux v2016.3
```
$ source petalinux/settings.sh
$ cd bsp/AXIOM-ZU9EG-2016.3
$ petalinux-config --verbose -c kernel
$ petalinux-build --verbose
$ petalinux-package --boot
```
**WARNING:** sourcing 'petalinux/settings.sh' inserts into the environment a
toolchain that does not work to build the AXIOM Software Stack. You must clean
the environment to build the AXIOM Software Stack.

**WARNING:** petalinux suggest to use bash as default system shell
```
$ sudo ln -sf /bin/bash /bin/sh
```

## 2. Download the code

### 2.1 Clone the repository
```
$ git clone https://github.com/evidence/axiom-evi.git
$ cd axiom-evi
```

### 2.2 Init the sub-modules (may take awhile)
```
$ ./scripts/submodules_update.sh
```

## 3. AXIOM Software Stack compilation

This section explains how to configure and compile the AXIOM sub-modules.

**Requirements:**

Note that some packages required for a build can be missing.
For example into a Ubuntu 16.04 LTS you should have installed the following packages:
```
$ sudo apt-get install build-essential git libssl-dev libncurses5-dev gawk bison wget
$ sudo apt-get install flex bc pkg-config libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev
$ sudo apt-get install linux-tools-generic linux-headers-generic autoconf automake gfortran libxml2-dev
$ sudo apt-get install libtool-bin libsqlite3-dev gperf debhelper fakeroot fakechroot qemu-user-static
$ sudo apt-get install automake autoconf libtool

```

### 3.1 Prepare the system

Change the environment variables into ```axiom-evi/settings.sh```.

* PETALINUX
    + You must have installed PetaLinux 2016.3; this must point to the installation directory
    + (default ```$HOME/petalinux```)

* AXIOMBSP
    + You must have installed SECO Petalinux project BSP; this must point to the BSP directory
    + (default ```$HOME/bsp/AXIOM-ZU9EG-2016.3```)

* LINARO
    + You must have a linaro toolchain; this must point to the linaro directory
    + (default $HOME/linaro)
    + You should use these linaro packages:
        * gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu.tar.xz
        * runtime-gcc-linaro-5.3.1-2016.05-aarch64-linux-gnu.tar.xz
        * sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu.tar.xz
    + The linaro installation directory must have the following layout:
```
            ${LINARO}/
            ${LINARO}/runtime-gcc-linaro-5.3.1-2016.05-aarch64-linux-gnu
            ${LINARO}/sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu
            ${LINARO}/gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu
            ${LINARO}/host (link to gcc-linaro-5.3.1-2016.05-x86_64_aarch64-linux-gnu)
            ${LINARO}/sysroot (link to sysroot-glibc-linaro-2.21-2016.05-aarch64-linux-gnu)
```
    + After the installation you must create a symbolic link "usr" to linaro/host directory:
```
            cd ${LINARO}/host
            ln -s . usr
```
    + Then you must create a link "aarcha64-linux-gnu-pkg-config" to pkg-config:
```
            cd ${LINARO}/host/usr/bin
            ln -s /usr/bin/pkg-config aarcha64-linux-gnu-pkg-config
```

* ROOTFS
    + You must have extracted the Seco Ubuntu file-system image; this must point to the installation directory
    + (default $HOME/zynq-rootfs)
    + Usually $HOME/zynq-rootfs is a link to $HOME/zynq-ubuntu-minimal_16.04-xenial_v1.3

    + NOTE: in zynq-rootfs/usr/lib/aarch64-linux-gnu there are .so with absolute link.
    Extrae try to use libdl.so (link to /lib/aarch64-linux-gnu/libdl.so.2)
    To fix this issue:
```
        cd zynq-rootfs/usr/lib/aarch64-linux-gnu
        rm libdl.so
        ln -s ../../../lib/aarch64-linux-gnu/libdl.so.2 libdl.so
```

* ROOTFSARCH
     + You must have a copy of the rootfs archive
     + (default $HOME/zynq-ubuntu-minimal_16.04-xenial_v1.3.tar.gz)

### 3.2 Compile the system

#### 3.2.1 Include settings file

To compile the system you must include the settings.sh into your environment, before to use the Makefile

```
    $ source axiom-evi/settings.sh
```

**Note** that the environment must be clean (i.e. you can not include the petalinux's settings)

#### 3.2.2 Define ```make``` variables

If you go into the ```scripts``` directory and do the command ```make``` for the first time, an help will be show:
```
    $ cd scripts
    $ make
    configure.mk:113: *** Command line environment variables not defined!
    You must define some variable on the command line (for example "make MODE=aarch64"):
    * MODE (required)
       aarch64: use linaro to cross-compile for arm 64bit
       x86: use native x86 kernel on rootfs
    * DISABLE_INSTR (optional, used during libraries compilation)
       0: compile EXTRAE and nic user library with instrumentation support (default)
       1: disable EXTRAE compilation and nic user library instrumentation
    * KVERSION (optional, used to compile X86 in a chroot)
       you must defined this if you are compiling for X86 from a chroot and must be
       set to the kernel version (otherwise a "uname -r" is used using the chroot host
       kernel version)
    * DFLAGS (optional, other gcc parameters to add during compilation
    Note that:
    MODE and DISABLE_INSTR must defined during "make configure" and never changed.
    KVERSION should be defined only for X86 chroot compilation during "make configure".
).  Stop.

```

#### 3.2.3 Configure the system

```
    $ make MODE=aarch64 DISABLE_INSTR=0 configure
```

you can change parameters as specified in the 3.2.2 section

#### 3.2.4 Compile the system

```
    $ make all
```


#### 3.2.5 Generate DEB packages

```
    $ make deb
```
The debian packages are generated into the ```axiom-evi/debs``` directory.

```
    $ make deb-tests
```
Generate the tests debian packages.


### 3.3 Handle temp overlay rootfs

During the compilation, we need some overlay rootfs to build the packages.
To umount these directories:

```
    $ make umount-temp
```

After the first compilation of all packages, if you want to compile in the
sub-module directory, you must mount these overlay roots:
```
    $ make mount-temp
```
**Note** that a temp overlay rootfs is mounted under ~/axiom-evi/rootfs-tmp and
is used to install the sub-projects applications/libraries; this rootfs must be
mounted if you use the make command from the sub-projects; this can be
accomplished using the following command from the ~/axiom-evi/scripts directory


### 3.4 Update the sub-modules

The best way to compile a new release is to clear the previous compilation to
be sure to have a clean state and update each sub-modules.

```
    $ cd axiom-evi/scripts
    $ make mrproper
    $ ./submodules_pull.sh
    $ make MODE=XXX DISABLE_INSTR=XXX configure
    $ make all
```

### 3.4 Note for x86 compilation

* the user that compile the software must be into the sudoers file (but it is
  not necessary to use a "sudo make .....")
* the system shell should be bash, so there must be a link from /bin/sh to bash
* the linux-tools-generic and linux-headers-generic packages must be installed

## 4. Note to compile an AXIOM application

### 4.1 Standalone application

You must link your application using the supplied linker script.
(This script include some entry point requested by the axiom memory manager).
You should add something like this into the makefile:
```
LDFLAGS += -Wl,-T~/axiom-evi/axiom-evi-allocator-lib/ldscript/aarch64/xs_map64.lds
```
or, if you are compiling on the board,
```
LDFLAGS += -Wl,-T/usr/axiom-evi-allocator-lib/xs_map64.lds
```
You could also use pkg-config:
```
pkg-config --libs evi_lmm
```

### 4.2 OMPSS@Cluster application

To compile a OMPSS@Cluster application you should use these flags for mercurium
```
MCCFLAGS := --ompss --cluster
```
during compilation. You must use the following flags during the link phase
```
LDFLAGS += --rdynamic -Wl,-T~/axiom-evi/axiom-evi-allocator-lib/ldscript/aarch64/xs_map64.lds
```
**NOTE** The ```--rdynamic``` flag is required otherwise the symbols from the supplied linker script will be discharged.

## 5 Note

### 5.1 OMPSS@Cluster

#### 5.1.1 Command line options

A new command-line options has been added: ```--use-my-cpus-number``` and can be zero (default) or one.
It is used to force the use of ```--smp-cpus``` parameters because, otherwise, this parameter can be set to a value greater than the number of cores of the microprocessor.

Example:
```
$ export NX_ARGS="--num-sockets=1 --cpus-per-socket=4 --smp-cpus=4"
```
this is the default for an AXIOM board if no parameter are passed. OMPSS create 4 physical thread and assign each to a core of the microprocessor then assign 4 working thread to these physical threads.

```
$ export NX_ARGS="--num-sockets=1 --cpus-per-socket=5 --smp-cpus=5 --use-my-cpu-number=1"
```
this create one more physical thread that share a core of the microprocessor and use it for a working thread.

```
$ export NX_ARGS="--num-sockets=1 --cpus-per-socket=5 --smp-cpus=5 --use-my-cpu-number=1 --cluster --cluster-network=axiom --cluster-smp-presend=4"
```
this create a physical thread for communication that share a core with 1 of the 4  working thread on each board.

#### 5.1.2 Scheduling policy

When you run a OMPSS@Cluster application you can define some environment variable to change
the scheduling policy of the system threads: ```AXIOM_WRK_PARAMS``` (for the working threads) and ```AXIOM_COM_PARAMS``` (for the communication threads)

The general form of the assignment is:
```
schedpol,par0,par1,par2
```
in which ```par0```, ```par1``` and ```par2``` are optional and depend on  ```schedpol```.

```schedpol``` can be:

* OTHER using the default linux policy
    + ```par0``` is the nice value (default to 0)
* FIFO using the FiFo real-time policy
    + ```par0``` is the priority (default to 1)
* RR using the RoundRobin real-time policy
    + ```par0``` if the priority (default to 1)
* BATCH using Batch policy, no parameters
* IDLE using the Idle policy, no parameters
* DEADLINE using the Deadline scheduling policy
    + ```par0``` is the execution time
    + ```par1``` is the deadline time
    + ```par2``` is the period time
    
    the time is specified using a units:
    
    + ```us``` micro-seconds
    + ```ms``` milli-seconds
    +  seconds otherwise

    the ```par0``` can also be specified using ```%``` of the period time

Examples:
```
$ export AXIOM_WRK_PARAMS=RR,2
$ export AXIOM_COM_PARRAMS=DEADLINE,10ms,80ms,100ms
```
or
```
$ export AXIOM_WRK_PARAMS=OTHER,10
$ export AXIOM_COM_PARRAMS=DEADLINE,2%,800us,10ms
```

