
export KERN
export FS
export QEMU_SNAP
export DISABLE_INSTR

_ccred:="\033[0;31m"
ccred:=$(shell echo -e $(_ccred))
ccgreen:=$(shell echo -e "\033[0;32m")
ccblue:=$(shell echo -e "\033[0;34m")
_ccend:="\033[0m"
ccend:=$(shell echo -e $(_ccend))

define ENV_HELP =
$(ccred)Command line enviroment variables not defined!$(ccend)
You must define some variable on the command line (for example "make P=1 E=0 run"):
* KERN ($(ccgreen)required$(ccend))
    br: use buildroot kernel
    seco: use seco bsp kernel (actually qemu does not work with this kernel)
* FS ($(ccgreen)optional$(ccend))
    br: use buildroot root fs
    seco: use seco (ubuntu xenial) root fs (default)
* QEMU_SNAP ($(ccgreen)optional$(ccend), used only during qemu run)
    0: the rootfs in on a static qcow2 (one for every machine)
    1: the rootfs in on a temp qcow2 snapshot (default)
* DISABLE_INSTR ($(ccgreen)optional$(ccend), used during libraries compilation)
    0: compile EXTRAE and nic user library with instrumentation support (default)
    1: disable EXTRAE compilation and nic user library instrumentation
)
endef

ifndef KERN
$(error $(ENV_HELP))
endif

# safe defaults
ifndef FS
FS := seco
endif
ifndef QEMU_SNAP
QEMU_SNAP := 1
endif
ifndef DISABLE_INSTR
DISABLE_INSTR := 0
endif

# check vars
ifneq ($(KERN),br)
ifneq ($(KERN),seco)
$(error KERN must be 'seco' or 'br')

endif
endif
ifneq ($(FS),br)
ifneq ($(FS),seco)
$(error FS must be 'seco' or 'br')
endif
endif
ifneq ($(QEMU_SNAP),0)
ifneq ($(QEMU_SNAP),1)
$(error QEMU_SNAP must be '0' or '1')
endif
endif
ifneq ($(DISABLE_INSTR),0)
ifneq ($(DISABLE_INSTR),1)
$(error DISABLE_INSTR must be '0' or '1')
endif
endif

include locations.mk
export OUTPUT_DIR

# safety
ifeq ($(KERN),seco)
ifeq ($(PETALINUX),)
$(error $(ccred)You must set the environment to work with KERN=seco (see settings.sh)$(ccend))
endif
endif
ifeq ($(FS),seco)
ifeq ($(ROOTFS),)
$(error $(ccred)You must set the environment to work with FS=seco (see settings.sh)$(ccend))
endif
endif

#
#
#

$(info Configuration:)
ifeq ($(KERN),seco)
$(info $(ccgreen)KERN$(ccend)=seco (using seco bsp kernel))
else
$(info $(ccgreen)KERN$(ccend)=br (using buildroot kernel))
endif
ifeq ($(FS),seco)
$(info $(ccgreen)FS$(ccend)=seco (using seco ubunto xenial rootfs))
else
$(info $(ccgreen)FS$(ccend)=br (using buildroot rootfs))
endif
ifeq ($(QEMU_SNAP),0)
$(info $(ccgreen)QEMU_SNAP$(ccend)=0 (using static qcow2 image for qemu run))
else
$(info $(ccgreen)QEMU_SNAP$(ccend)=1 (using temp qcow2 image for qemu run))
endif
ifeq ($(DISABLE_INSTR),0)
$(info $(ccgreen)DISABLE_INSTR$(ccend)=0 (using EXTRAE instrumentation))
else
$(info $(ccgreen)DISABLE_INSTR$(ccend)=1 (disable EXTRAE compilation and integration))
endif

#
# archive/release definitions/parameters
#

DATE := $(shell date +%Y%m%d_%H%M%S)-
GIT_DESC := $(shell git describe --tags | sed s/axiom-//g)
ARCHIVE_VMS_TGZ := $(DATE)axiom-nic-evisim-$(GIT_DESC)
ARCHIVE_TOOLCHAIN_TGZ := $(DATE)axiom-evi-toolchain-$(GIT_DESC)
release: ARCHIVE_VMS_TGZ := axiom-nic-evisim-$(GIT_DESC)
release: ARCHIVE_TOOLCHAIN_TGZ := axiom-evi-toolchain-$(GIT_DESC)

#
# misc definitions/parameters
#

LOG_FILE := make.log

#
# start/run definitions/parameters
#

# default number of Virtual Machines to run
VMS := 2

# qemu

QEMU := $(QEMU_BUILD_DIR)/aarch64-softmmu/qemu-system-aarch64
QEMU-STATIC := $(QEMU_STATIC_BUILD_DIR)/aarch64-linux-user/qemu-aarch64
QEMU-IMG := $(QEMU_BUILD_DIR)/qemu-img
QEMU-NBD := /usr/bin/qemu-nbd
QEMU_PARAM := "--longenq"
ifeq ($(LONG_ENQUEUE), 0)
QEMU_PARAM := ""
endif
ifeq ($(FS),seco)
QEMU_PARAM += "--petalinux"
endif
ifeq ($(QEMU_SNAP),1)
QEMU_PARAM += "--snapshot"
endif

# axiom switch

SWITCH :=$(EVI_NIC_DIR)/axiom_switch/axiom_switch
SWITCH_PARAM := -r -n $(VMS)
ifdef SWITCH
SWITCH_RUN := $(SWITCH) $(SWITCH_PARAM) >> $(LOG_FILE) 2>&1 &
else
SWITCH_RUN :=
SWITCH_PARAM :=
endif

#
# compile/build definitions/parameters
#

DFLAGS := -g
debug: DFLAGS := -g -DPDEBUG
export DFLAGS

#CCARCH := arm
CCARCH := aarch64
ifeq ($(FS),seco)
CCPREFIX := ${LINARO}/host/usr/bin/$(CCARCH)-linux-gnu-
HOSTID:=aarch64-linux-gnu
LINPATH:=$(LINARO)/host/usr/bin:$(PATH)
else
CCPREFIX := ${OUTPUT_DIR}/host/usr/bin/$(CCARCH)-linux-
HOSTID:=aarch64-buildroot-linux-gnu
BDPATH := $(OUTPUT_DIR)/host/usr/bin/:$(PATH)
LINPATH := $(PATH)
endif

# fakeroot

FAKEROOT :=
ifeq ($(FS),seco)
ifndef FAKEROOTKEY
FAKEROOT := fakeroot -i $(ROOTFS).faked -s $(ROOTFS).faked
endif
endif

#
# makefile definitions/parameters
#

IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))
SYNCS := $(addprefix sync,$(IDS))

BUILDCPUS := $(shell getconf _NPROCESSORS_ONLN)

#
# MAIN targets
#

.PHONY: all debug target

ifeq ($(FS),seco)
all: axiom-nic axiom-apps qemu gasnet ompss tests
else
all: axiom-nic axiom-apps arm-firmware qemu gasnet ompss tests target
endif

debug: all

target: _buildroot arm-firmware qemu

help::
	@echo "Makefile targets"
	@echo " "

help::
	@echo "GENERAL"
	@echo "  all    Build all libraries, applications and create a root fs"
	@echo "  debug  Same as 'all' but with DFLAGS='-g -DPDEBUG'"
	@echo "  target Create a root fs"
	@echo "  help   This help"
	@echo " "

#
# CONFIGURE
#

.PHONY: configure config-alloc config-buildroot

configure: config-alloc
ifeq ($(FS),seco)
	mkdir -p $(QEMU_STATIC_BUILD_DIR)
	cd $(QEMU_STATIC_BUILD_DIR) && ../axiom_configure_static.sh
endif
	mkdir -p $(QEMU_BUILD_DIR)
	cd $(QEMU_BUILD_DIR) && ../axiom_configure.sh

config-alloc: config-buildroot
	cd $(EVI_ALLOC_LIB_DIR) &&  autoreconf -fi
	cd $(ALLOCATOR_DIR) && autoreconf -fi

config-buildroot:
ifneq ($(FS),seco)
	cd $(BUILDROOT_DIR) && $(MAKE) O=$(OUTPUT_DIR) evi_ultrascale_defconfig && $(MAKE) O=$(OUTPUT_DIR)
endif

help::
	@echo "CONFIGURE"
	@echo "  configure          Configure the subrojects (and build toolchain)"
	@echo "  "
	@echo "  config-alloc       Configure memory allocator"
	@echo "  config-buildroot   Build buildroot toolchain"
	@echo " "

#
# BUILD target filesystem
#

.PHONY: libs allocator-driver allocator-lib allocator-lib-config allocator-lib-config-force allocator axiom-nic axiom-apps _buildroot buildroot arm-firmware qemu-dts qemu allocator allocator-config-force allocator-config

ifeq ($(DISABLE_INSTR),1)
libs: 
else
libs: extrae 
endif
	$(MAKE) -C $(EVI_NIC_DIR) libs
	$(MAKE) -C $(EVI_APPS_DIR) libs

allocator-driver: libs
	$(MAKE) -C $(EVI_ALLOC_DRV_DIR)
	$(FAKEROOT) $(MAKE) DESTDIR=$(TARGET_DIR) -C $(EVI_ALLOC_DRV_DIR) install
ifneq ($(FS),seco)
	$(MAKE) DESTDIR=$(SYSROOT_DIR) -C $(EVI_ALLOC_DRV_DIR) install
endif

allocator-lib-bootstrap:
	@if [ ! -r $(EVI_ALLOC_LIB_DIR)/configure ]; then \
		cd $(EVI_ALLOC_LIB_DIR);\
		autoreconf -fiv ;\
	fi

allocator-lib-config-force: allocator-lib-bootstrap
ifeq ($(FS),seco)
	cd $(EVI_ALLOC_LIB_DIR) && PATH=$(LINPATH) ./configure \
		--host=$(HOSTID) --prefix=/usr --disable-debug --enable-extld
else
	cd $(EVI_ALLOC_LIB_DIR) && CC=$(CCPREFIX)gcc ./configure \
		--host=$(HOSTID) --prefix=/usr --disable-debug --enable-extld
endif

allocator-lib-config: libs
	if [ ! -r "$(EVI_ALLOC_LIB_DIR)/config.status" ]; then $(MAKE) allocator-lib-config-force; fi

allocator-lib: allocator-lib-config
	PATH=$(LINPATH) $(MAKE) -C $(EVI_ALLOC_LIB_DIR)
	$(FAKEROOT) $(MAKE) DESTDIR=$(TARGET_DIR) -C $(EVI_ALLOC_LIB_DIR) install
ifneq ($(FS),seco)
	$(MAKE) DESTDIR=$(SYSROOT_DIR) -C $(EVI_ALLOC_LIB_DIR) install
endif
        # libtool fix :-(
	for LIB in $(SYSROOT_DIR)/usr/lib/libevi_lmm.la $(TARGET_DIR)/usr/lib/libevi_lmm.la; do \
		$(FAKEROOT) sed -i -e "s,libdir='/usr/lib',libdir='$(SYSROOT_DIR)/usr/lib'," \
		       -e "s, /usr/lib, $(SYSROOT_DIR)/usr/lib," \
		       $${LIB} ;\
	done

allocator-config-force:
ifeq ($(FS),seco)
	cd $(ALLOCATOR_DIR) && PATH=$(LINPATH) ./configure \
		--host=$(HOSTID) --prefix=/usr --with-axiom-sysroot=$(ROOTFS)
else
	cd $(ALLOCATOR_DIR) && CC=$(CCPREFIX)gcc ./configure \
		--host=$(HOSTID) --prefix=/usr
endif

allocator-config: allocator-driver allocator-lib
	if [ ! -r "$(ALLOCATOR_DIR)/config.status" ]; then $(MAKE) allocator-config-force; fi

allocator: allocator-config
	PATH=$(LINPATH) $(MAKE) -C $(ALLOCATOR_DIR)
	PATH=$(LINPATH) $(FAKEROOT) $(MAKE) DESTDIR=$(TARGET_DIR) -C $(ALLOCATOR_DIR) install
ifneq ($(FS),seco)
	$(MAKE) DESTDIR=$(SYSROOT_DIR) -C $(ALLOCATOR_DIR) install
endif
        # libtools fix :-(
	for LIB in $(SYSROOT_DIR)/usr/lib/libaxiom_allocator*.la; do \
		$(FAKEROOT) sed -i -e "s,libdir='/usr/lib',libdir='$(SYSROOT_DIR)/usr/lib'," \
		       -e "s, /usr/lib, $(SYSROOT_DIR)/usr/lib," \
		       $${LIB} ;\
	done

axiom-nic: libs allocator
	$(MAKE) -C $(EVI_NIC_DIR) install

axiom-apps: libs allocator
	$(MAKE) -C $(EVI_APPS_DIR) install

_buildroot:
	$(MAKE) -C $(OUTPUT_DIR)

#buildroot: axiom-apps axiom-nic
#	$(MAKE) _buildroot

arm-firmware:
	$(MAKE) -C $(ARM_FIRMWARE_DIR) CROSS_COMPILE=$(CCPREFIX) RESET_TO_BL31=1 PLAT=zynqmp all fip


$(QEMU_DTS_DIR)/axiom-board.dtb $(QEMU_DTS_DIR)/axiom-board-long-enqueue.dtb:
	$(MAKE) KERN=br qemu-dts

qemu-dts:
ifneq ($(KERN),seco)
	$(MAKE) -C $(QEMU_DTS_DIR) DTC=$(HOST_DIR)/usr/bin/linux-dtc
endif

qemu: qemu-dts
ifeq ($(FS),seco)
	$(MAKE) -C $(QEMU_STATIC_BUILD_DIR) -j$(BUILDCPUS)
endif
	$(MAKE) -C $(QEMU_BUILD_DIR) -j$(BUILDCPUS)

help::
	@echo "BUILD"
	@echo "  libs               Build axiom API libraries"
	@echo "  allocator-driver   Build Evidence kernel allocator driver"
	@echo "  allocator-lib      Build Evidence memory allocator libraries"    
	@echo "  allocator          Build memory allocator"
	@echo "  axiom-nic          Build axiom kernel driver (and, eventually, user API libs)"
	@echo "  axiom-apps         Build axiom user applications (and, eventually, run/init API libs)"
	@echo "  buildroot          Build target filesystem"
	@echo "  arm-firmware       Build arm firmware"
	@echo "  qemu-dts           Build system DTS"
	@echo "  qemu               Build QEMU"
	@echo " "
	@echo "  _buildroot                 Build ONLY buildroot target filesystem"
	@echo "  allocator-lib-config       Configure the allocator libraries"
	@echo "  allocator-lib-config-force Force the configuration of the allocator libraries"
	@echo "  allocator-config           Configure the allocator"
	@echo "  allocator-config-force     Force the configuration of the allocato"
	@echo " "

#
# GASNET
#

.PHONY: gasnet-bootstrap gasnet-debug-configure gasnet-debug gasnet-perf-config gasnet-perf gasnet clean-gasnet distclean-gasnet

gasnet-bootstrap:
	@if [ ! -r $(GASNET_DIR)/configure ]; then \
		cd $(GASNET_DIR);\
		./Bootstrap;\
	fi

gasnet-config-debug: gasnet-bootstrap libs
	mkdir -p $(GASNET_BUILD_DIR_DEBUG)
	@if [ ! -r "$(GASNET_BUILD_DIR_DEBUG)/config.status" ]; then \
		cd $(GASNET_BUILD_DIR_DEBUG); ../cross-configure-axiom \
		--with-axiom-host=$(HOST_DIR) \
		--with-axiom-sysroot=$(SYSROOT_DIR) \
		--prefix=/usr \
		--libdir=/usr/lib/debug \
		--includedir=/usr/include/debug \
		--enable-udp \
		--enable-debug \
		--enable-trace --enable-backtrace;\
	fi

gasnet-debug: gasnet-config-debug
	@$(MAKE) -C $(GASNET_BUILD_DIR_DEBUG)
ifeq ($(FS),seco)
	$(FAKEROOT) $(MAKE) -C $(GASNET_BUILD_DIR_DEBUG) DESTDIR=$(TARGET_DIR) install
else
	@$(MAKE) -C $(GASNET_BUILD_DIR_DEBUG) DESTDIR=$(SYSROOT_DIR) install
endif

gasnet-config-perf: gasnet-bootstrap libs
	mkdir -p $(GASNET_BUILD_DIR_PERF)
	if [ ! -r "$(GASNET_BUILD_DIR_PERF)/config.status" ]; then \
		cd $(GASNET_BUILD_DIR_PERF); ../cross-configure-axiom \
		--with-axiom-host=$(HOST_DIR) \
		--with-axiom-sysroot=$(SYSROOT_DIR) \
		--prefix=/usr \
		--libdir=/usr/lib/performance \
		--includedir=/usr/include/performance \
		--enable-udp \
		--disable-debug ;\
	fi

gasnet-perf: gasnet-config-perf
	@$(MAKE) -C $(GASNET_BUILD_DIR_PERF)
ifeq ($(FS),seco)
	$(FAKEROOT) $(MAKE) -C $(GASNET_BUILD_DIR_PERF) DESTDIR=$(TARGET_DIR) install
else
	@$(MAKE) -C $(GASNET_BUILD_DIR_PERF) DESTDIR=$(SYSROOT_DIR) install
endif

gasnet: gasnet-debug gasnet-perf

clean-gasnet:
	@[ -d $(GASNET_BUILD_DIR_DEBUG) ] && $(MAKE) -C $(GASNET_BUILD_DIR_DEBUG) $(subst -gasnet,,$@)
	@[ -d $(GASNET_BUILD_DIR_PERF) ] && $(MAKE) -C $(GASNET_BUILD_DIR_PERF) $(subst -gasnet,,$@)

distclean-gasnet:
	@[ -d $(GASNET_BUILD_DIR_DEBUG) ] && rm -rf $(GASNET_BUILD_DIR_DEBUG) || true
	@[ -d $(GASNET_BUILD_DIR_PERF) ] && rm -rf $(GASNET_BUILD_DIR_PERF) || true
	@if [ -r $(GASNET_DIR)/configure ]; then \
		cd $(GASNET_DIR);\
		./unBootstrap;\
	fi

help::
	@echo "GASNET"
	@echo "  gasnet               Build all gasnet libraries"
	@echo "  clean-gasnet         Clean gasnet libraries"
	@echo "  distclean-gasnet     Clean gasnet libraries including configuration"
	@echo "  "
	@echo "  gasnet-boostrap      Gasnet configure bootstrap"
	@echo "  gasnet-config-debug  Configuration for debug libraries"
	@echo "  gasnet-config-perf   Configuration for performance libraries"
	@echo "  gasnet-debug         Build debug library"
	@echo "  gasnet-perf          Build performace library"
	@echo " "

#
# NANOX
#

.PHONY: nanox-bootstrap nanox-config-debug nanox-config-debug-force nanox-debug nanox-config-perf nanox-config-perf-force nanox-perf nanox clean-nanox distclean-nanox

NANOXFLAGS := CXXFLAGS="-std=c++11"
NANOXFLAGS_INSTR := $(NANOXFLAGS)
ifeq ($(P),1)
NANOXFLAGS_INSTR := CXXFLAGS="-std=c++11 --sysroot=$(ROOTFS) -I$(ROOTFS)/usr/include/aarch64-linux-gnu"
endif

nanox-bootstrap:
	@if [ ! -r $(NANOX_DIR)/configure ]; then \
		cd $(NANOX_DIR);\
		./bootstrap;\
	fi

nanox-config-debug-force: nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_DEBUG)
	@cd $(NANOX_BUILD_DIR_DEBUG); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr $(NANOXFLAGS) \
		../scripts/cross-configure-axiom_debug.sh

nanox-config-debug:
	@if [ ! -r "$(NANOX_BUILD_DIR_DEBUG)/config.status" ]; then $(MAKE) nanox-config-debug-force; fi

nanox-debug: nanox-config-debug
	@PATH=$(LINPATH) $(MAKE) $(NANOXFLAGS) -j$(BUILDCPUS) -C $(NANOX_BUILD_DIR_DEBUG)
	@PATH=$(LINPATH) $(FAKEROOT) $(MAKE) -C $(NANOX_BUILD_DIR_DEBUG) DESTDIR=$(TARGET_DIR) install
ifneq ($(FS),seco)
	@$(MAKE) -C $(NANOX_BUILD_DIR_DEBUG) DESTDIR=$(SYSROOT_DIR) install
endif

nanox-config-perf-force: nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_PERF)
	@cd $(NANOX_BUILD_DIR_PERF); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr $(NANOXFLAGS) \
		../scripts/cross-configure-axiom_performance.sh

nanox-config-perf:
	@if [ ! -r "$(NANOX_BUILD_DIR_PERF)/config.status" ]; then $(MAKE) nanox-config-perf-force; fi

nanox-perf: nanox-config-perf
	@PATH=$(LINPATH) $(MAKE) $(NANOXFLAGS) -j$(BUILDCPUS) -C $(NANOX_BUILD_DIR_PERF)
	@PATH=$(LINPATH) $(FAKEROOT) $(MAKE) -C $(NANOX_BUILD_DIR_PERF) DESTDIR=$(TARGET_DIR) install
ifneq ($(FS),seco)
	@$(MAKE) -C $(NANOX_BUILD_DIR_PERF) DESTDIR=$(SYSROOT_DIR) install
endif

nanox-config-instr-force: extrae nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_INSTR)
	@cd $(NANOX_BUILD_DIR_INSTR); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr $(NANOXFLAGS) \
		../scripts/cross-configure-axiom_instrumentation.sh

nanox-config-instr:
	@if [ ! -r "$(NANOX_BUILD_DIR_INSTR)/config.status" ]; then $(MAKE) nanox-config-instr-force; fi

nanox-instr: nanox-config-instr
	PATH=$(LINPATH) $(MAKE) $(NANOXFLAGS_INSTR) -j$(BUILDCPUS) -C $(NANOX_BUILD_DIR_INSTR)
	@PATH=$(LINPATH) $(FAKEROOT) $(MAKE) -C $(NANOX_BUILD_DIR_INSTR) DESTDIR=$(TARGET_DIR) install
ifneq ($(FS),seco)
	@$(MAKE) -C $(NANOX_BUILD_DIR_INSTR) DESTDIR=$(SYSROOT_DIR) install
endif

nanox-config-instr-debug-force: extrae nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_INSTR_DEBUG)
	@cd $(NANOX_BUILD_DIR_INSTR_DEBUG); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr $(NANOXFLAGS) \
		../scripts/cross-configure-axiom_instrumentation_debug.sh

nanox-config-instr-debug:
	@if [ ! -r "$(NANOX_BUILD_DIR_INSTR_DEBUG)/config.status" ]; then $(MAKE) nanox-config-instr-debug-force; fi

nanox-instr-debug: nanox-config-instr-debug
	@PATH=$(LINPATH) $(MAKE) $(NANOXFLAGS) -j$(BUILDCPUS) -C $(NANOX_BUILD_DIR_INSTR_DEBUG)
	@PATH=$(LINPATH) $(FAKEROOT) $(MAKE) -C $(NANOX_BUILD_DIR_INSTR_DEBUG) DESTDIR=$(TARGET_DIR) install
ifneq ($(FS),seco)
	@$(MAKE) -C $(NANOX_BUILD_DIR_INSTR_DEBUG) DESTDIR=$(SYSROOT_DIR) install
endif

nanox: nanox-debug nanox-perf nanox-instr

clean-nanox:
	@[ -d $(NANOX_BUILD_DIR_DEBUG) ] && make -C $(NANOX_BUILD_DIR_DEBUG) clean
	@[ -d $(NANOX_BUILD_DIR_PERF) ] && make -C $(NANOX_BUILD_DIR_PERF) clean
	@[ -d $(NANOX_BUILD_DIR_INSTR) ] && make -C $(NANOX_BUILD_DIR_INSTR) clean

distclean-nanox:
	@[ -d $(NANOX_BUILD_DIR_DEBUG) ] && rm -rf $(NANOX_BUILD_DIR_DEBUG) || true
	@[ -d $(NANOX_BUILD_DIR_PERF) ] && rm -rf $(NANOX_BUILD_DIR_PERF) || true
	@[ -d $(NANOX_BUILD_DIR_INSTR) ] && rm -rf $(NANOX_BUILD_DIR_INSTR) || true

help::
	@echo "NANOX (OMPSS)"
	@echo "  nanox               Build Nanos++ libraries"
	@echo "  clean-nanox         Clean Nanos++ libraries"
	@echo "  distclean-nanox     Clean Nanos++ libraries including configuration"
	@echo " "
	@echo "  nanox-bootstrap          Configuration bootsrap"
	@echo "  nanox-config-debug       Configure debug library"
	@echo "  nanox-config-perf        Configure performace library"
	@echo "  nanox-config-instr       Configure performace library with Extrae"
	@echo "  nanox-debug              Build debug library"
	@echo "  nanox-perf               Build performance library"
	@echo "  nanox-instr         	  Build performance library with Extrae"
	@echo "  nanox-config-debug-force Force the configuration of nanox debug"
	@echo "  nanox-config-perf-force  Force the configuration of nanox performance"
	@echo "  nanox-config-instr-force Force the configuration of nanox instrumentation"
	@echo " "

#
# MCXX
#

.PHONY: mcxx-boostrap mcxx-config mcxx-config-force mcxx clean-mcxx distclean-mcxx

mcxx-bootstrap:
	@if [ ! -r $(MCXX_DIR)/configure ]; then \
		cd $(MCXX_DIR);\
		autoreconf -fiv;\
	fi

mcxx-config-force: nanox mcxx-bootstrap
	@mkdir -p $(MCXX_BUILD_DIR)
	cd $(MCXX_BUILD_DIR); AXIOMHOME=$(AXIOMHOME) PREFIX=$(HOST_DIR)/usr \
		../scripts/cross-configure-axiom.sh

mcxx-config:
	@if [ ! -r "$(MCXX_BUILD_DIR)/config.status" ]; then $(MAKE) mcxx-config-force; fi

mcxx: mcxx-config
	@make -j$(BUILDCPUS) PATH=$(PATH):$(HOST_DIR)/usr/bin -C $(MCXX_BUILD_DIR)
#	@make -C $(MCXX_BUILD_DIR) DESTDIR=$(HOST_DIR) install
	@make -C $(MCXX_BUILD_DIR) install
ifneq ($(FS),seco)
	@cp $(HOST_DIR)/usr/lib/pkgconfig/mcxx.pc $(SYSROOT_DIR)/usr/lib/pkgconfig/mcxx.pc
endif

mcxx-native-config-force: nanox mcxx-bootstrap
	@mkdir -p $(MCXX_BUILD_NATIVE_DIR)
	cd $(MCXX_BUILD_NATIVE_DIR); AXIOMHOME=$(AXIOMHOME) PREFIX=$(HOST_DIR)/usr \
		../scripts/cross-configure-axiom-native.sh

mcxx-native-config:
	@if [ ! -r "$(MCXX_BUILD_NATIVE_DIR)/config.status" ]; then $(MAKE) mcxx-native-config-force; fi

# does not WORK!!!!!
mcxx-native: mcxx-native-config
	@make -j$(BUILDCPUS) \
		CFLAGS="--sysroot=$(ROOTFS) -I$(ROOTFS)/usr/include/aarch64-linux-gnu" \
		CXXFLAGS="--sysroot=$(ROOTFS) -I$(ROOTFS)/usr/include/aarch64-linux-gnu -std=gnu++11" \
		LDFLAGS="-L$(ROOTFS)/lib/aarch64-linux-gnu -L$(ROOTFS)/usr/lib//aarch64-linux-gnu" \
		PATH=$(PATH):$(HOST_DIR)/usr/bin \
		-C $(MCXX_BUILD_NATIVE_DIR)
	@make -C $(MCXX_BUILD_NATIVE_DIR) install
ifneq ($(P),1)
	@cp $(HOST_DIR)/usr/lib/pkgconfig/mcxx.pc $(SYSROOT_DIR)/usr/lib/pkgconfig/mcxx.pc
endif

clean-mcxx:
	@[ -d $(MCXX_BUILD_DIR) ] && make -C $(MCXX_BUILD_DIR) clean

distclean-mcxx:
	@[ -d $(MCXX_BUILD_DIR) ] && rm -rf $(MCXX_BUILD_DIR) || true

help::
	@echo "MCXX (OMPSS)"
	@echo "  mcxx              Build Mercurium source cross compiler (run on HOST)"
	@echo "  mcxx-native       Build Mercurium source cross compiler (run on TARGET)"
	@echo "  clean-mcxx        Clean Mercurium"
	@echo "  distclean-mcxx    Clean Mercurium includind configuration"
	@echo ""
	@echo "  mcxx-bootstrap           Configuration bootstrap"
	@echo "  mcxx-config              Mercurium configuration"
	@echo "  mcxx-config-force        Force the configuration of Mercurium"
	@echo "  mcxx-native-config       Mercurium configuration (native build)"
	@echo "  mcxx-native-config-force Force the configuration of Mercurium (native build)"
	@echo " "

#
# EXTRAE
#

.PHONY: extrae-boostrap extrae-config-force extrae-config extrae clean-extrae distclean-extrae

#ifeq ($(P), 1)
#
#extrae-bootstrap:
#extrae-config-force: extrae-bootstrap
#extrae-config:
#extrae: extrae-config
#clean-extrae:
#distclean-extrae:
#	@[ -d $(EXTRAE_BUILD_DIR) ] && rm -rf $(EXTRAE_BUILD_DIR) || true
#
#else

extrae-bootstrap:
	@if [ ! -r $(EXTRAE_DIR)/configure ]; then \
		cd $(EXTRAE_DIR);\
		autoreconf -fiv;\
	fi

extrae-config-force: extrae-bootstrap
	@mkdir -p $(EXTRAE_BUILD_DIR)
	cd $(EXTRAE_BUILD_DIR); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr \
		../scripts/cross-configure-axiom.sh

extrae-config:
	@if [ ! -r "$(EXTRAE_BUILD_DIR)/config.status" ]; then $(MAKE) extrae-config-force; fi

extrae: extrae-config
ifeq ($(FS),seco)
	@PATH=$(LINPATH) $(MAKE) -j$(BUILDCPUS) -C $(EXTRAE_BUILD_DIR) \
	  CFLAGS="--sysroot=$(ROOTFS) \
		-I${ROOTFS}/usr/include/aarch64-linux-gnu \
		-L${ROOTFS}/usr/lib/aarch64-linux-gnu \
		-L${ROOTFS}/lib/aarch64-linux-gnu" \
	  LDFLAGS="--sysroot=$(ROOTFS) \
		-L${ROOTFS}/usr/lib/aarch64-linux-gnu \
		-L${ROOTFS}/lib/aarch64-linux-gnu \
		-Wl,-rpath-link -Wl,$(ROOTFS)/usr/lib/aarch64-linux-gnu \
		-Wl,-rpath-link -Wl,$(ROOTFS)/lib/aarch64-linux-gnu"
	@PATH=$(LINPATH) $(FAKEROOT) $(MAKE) -C $(EXTRAE_BUILD_DIR) DESTDIR=$(TARGET_DIR) install
else
	@$(MAKE) -j$(BUILDCPUS) -C $(EXTRAE_BUILD_DIR)
	@$(MAKE) -C $(EXTRAE_BUILD_DIR) DESTDIR=$(TARGET_DIR) install
	@$(MAKE) -C $(EXTRAE_BUILD_DIR) DESTDIR=$(SYSROOT_DIR) install
endif

clean-extrae:
	[ -d $(EXTRAE_BUILD_DIR) ] && make -C $(EXTRAE_BUILD_DIR) clean

distclean-extrae:
	[ -d $(EXTRAE_BUILD_DIR) ] && rm -rf $(EXTRAE_BUILD_DIR) || true

#endif

help::
	@echo "EXTRAE (OMPSS)"
	@echo "  extrae              Build Extrae library"
	@echo "  clean-extrae        Clean Extrae"
	@echo "  distclean-extrae    Clean Extrae includind configuration"
	@echo ""
	@echo "  extrae-bootstrap    Configuration bootstrap"
	@echo "  extrae-config-force Extrae configuration"
	@echo "  extrae-config       Force the configuration of Extrae"
	@echo ""

#
# OMPSS
#

.PHONY: ompss clean-ompss distclean-ompss

ompss: nanox mcxx

clean-ompss: clean-nanox clean-mcxx

distclean-ompss: distclean-nanox distclean-mcxx

help::
	@echo "OMPSS"
	@echo "  ompss             Build OmpSs (Nanos++ and Mercurium)"
	@echo "  clean-ompss       Clean OmpSs"
	@echo "  distclean-ompss   Clean OmpSS including configuration"
	@echo " "

#
# RUNNING
#

.PHONY: start run netup netdown setupnet stop switch log $(RUNS) sync $(SYNCS)

MASTERROOTFS_SECO_NAME := zynq-rootfs_seco
MASTERROOTFS_BR_NAME := zynq-rootfs_br

ifeq ($(FS),seco)
MASTERROOTFS_NAME := $(MASTERROOTFS_SECO_NAME)
MASTERROOTFS_SIZE := 4G
ROOTFSARCH_FILE := $(ROOTFSARCH)
UBOOT_FILE:=${IMAGES}/u-boot.sd.elf
else
MASTERROOTFS_NAME := $(MASTERROOTFS_BR_NAME)
MASTERROOTFS_SIZE := 2G
ROOTFSARCH_FILE := $(IMAGES)/rootfs.cpio
#UBOOT_FILE:=${IMAGES}/u-boot.elf
UBOOT_FILE:=${IMAGES}/u-boot.sd.elf
endif

MASTERROOTFS:=$(IMAGES)/$(MASTERROOTFS_NAME).qcow2
TMPMOUNTPOINT:=$(IMAGES)/tmp
BR_UBOOT_FILE:=${IMAGES}/u-boot.elf
UBOOT_PATCH:=$(AXIOMHOME)/scripts/buildroot/uboot-petalinux.patch

.PHONY: build-qemu-rootfs build-qemu-uboot

# create rootfs for qemu
#$(MASTERROOTFS):
#	@if [ ! -e "$(ROOTFSARCH_FILE)" ]; then \
#		echo "$(ROOTFSARCH_FILE) NOT FOUND!"; \
#		/bin/false; \
#	fi
#	mkdir -p $(IMAGES)
#	fallocate -l $(MASTERROOTFS_SIZE) $(MASTERROOTFS)
#	/sbin/mkfs.ext4 $(MASTERROOTFS)
#	mkdir -p $(TMPMOUNTPOINT)
#	sudo mount -t auto -o loop,rw $(MASTERROOTFS) $(TMPMOUNTPOINT)
#ifeq ($(P),1)
#	sudo tar -C $(TMPMOUNTPOINT) -xvzf $(ROOTFSARCH_FILE)
#else
#	sudo /bin/sh -c 'cd $(TMPMOUNTPOINT); cpio -i <$(ROOTFSARCH_FILE);'
#endif
#	sudo umount $(TMPMOUNTPOINT)

NBD:=/dev/nbd7

# create rootfs for qemu
$(MASTERROOTFS):
	@if [ ! -e "$(ROOTFSARCH_FILE)" ]; then \
		echo "$(ROOTFSARCH_FILE) NOT FOUND!"; \
		/bin/false; \
	fi
	mkdir -p $(IMAGES)
# create the image file
	$(QEMU-IMG) create -f qcow2 $(MASTERROOTFS) $(MASTERROOTFS_SIZE)
# mount the image
	sudo $(QEMU-NBD) -f qcow2 --connect=$(NBD) $(MASTERROOTFS)
	sudo /sbin/mkfs.ext4 -E nodiscard $(NBD)
	sudo mount -t auto -o defaults,rw $(NBD) $(TMPMOUNTPOINT)
# fill the image
ifeq ($(FS),seco)
	sudo tar -C $(TMPMOUNTPOINT) -xvzf $(ROOTFSARCH_FILE)
else
	sudo /bin/sh -c 'cd $(TMPMOUNTPOINT); cpio -i <$(ROOTFSARCH_FILE);'
endif
# copy qemu static interpreter
	-sudo cp $(QEMU_STATIC_BUILD_DIR)/aarch64-linux-user/qemu-aarch64 \
		$(TMPMOUNTPOINT)/usr/bin/
ifeq ($(FS),seco)
# copy the overlay dir and the debs
	-sudo cp -r $(UBUNTU_OVERLAY_DIR)/* $(TMPMOUNTPOINT)
	-sudo mkdir $(TMPMOUNTPOINT)/debs
	-sudo cp $(AXIOMHOME)/debs/* $(TMPMOUNTPOINT)/debs/
# some xenial debian packages
	-sudo cp $(XENIAL_DIR)/*.deb $(TMPMOUNTPOINT)/tmp/
	-sudo chroot $(TMPMOUNTPOINT) /bin/bash -c "dpkg --install /tmp/*.deb"
	-sudo rm -f $(TMPMOUNTPOINT)/tmp/*.deb
# setup debian archive
	-sudo /bin/bash -c \
		'echo "deb file:/// debs/" >$(TMPMOUNTPOINT)/etc/apt/sources.list.d/axiom'
# install debian packages
#	-sudo chroot $(TMPMOUNTPOINT) /bin/bash -c "apt-get update; apt-get install axiom"
	-sudo chroot $(TMPMOUNTPOINT) /bin/bash -c "dpkg --install /debs/*.deb"
endif
# remove qemu static interpreter
	-sudo rm -f $(TMPMOUNTPOINT)/usr/bin/qemu-aarch64
# umount the image
	sudo umount $(TMPMOUNTPOINT)
	sudo $(QEMU-NBD) --disconnect $(NBD)

# create u-boot for SD image rootfs
${IMAGES}/u-boot.sd.elf:
	cp -f $(OUTPUT_DIR)/build/uboot-axiom/include/configs/xilinx_zynqmp.h /tmp/
	cp -f $(BR_UBOOT_FILE) /tmp/
	-patch -p1 -d $(OUTPUT_DIR) <$(UBOOT_PATCH)
	-make -C $(OUTPUT_DIR) uboot-rebuild
	-cp -f $(BR_UBOOT_FILE) ${IMAGES}/u-boot.sd.elf
	cp -f /tmp/u-boot.elf $(BR_UBOOT_FILE)
	cp -f /tmp/xilinx_zynqmp.h \
		$(OUTPUT_DIR)/build/uboot-axiom/include/configs/xilinx_zynqmp.h 

# create a Packages.gz to use with apt-get
$(AXIOMHOME)/debs/Packages.gz: $(AXIOMHOME)/debs/*.deb
	dpkg-scanpackages $(AXIOMHOME)/debs |\
		sed 's,Filename:.*/,Filename: /debs/,' |\
		gzip -9 >$(AXIOMHOME)/debs/Packages.gz

# create permanent rootfs snapshot for qemu
$(IMAGES)/$(MASTERROOTFS_NAME)-snap-%.qcow2: $(MASTERROOTFS) $(AXIOMHOME)/debs/Packages.gz
	$(QEMU-IMG) create -f qcow2  $(IMAGES)/$(MASTERROOTFS_NAME)-snap-$*.qcow2 \
		-o backing_file=$(MASTERROOTFS)
# the image for petalinux/seco must be fixed!
ifeq ($(FS),seco)
# mount the image
	-sudo $(QEMU-NBD) -f qcow2 --connect=$(NBD) \
		$(IMAGES)/$(MASTERROOTFS_NAME)-snap-$*.qcow2
	-sudo mount -t auto -o defaults,rw $(NBD) $(TMPMOUNTPOINT)
	-sudo cp $(QEMU_STATIC_BUILD_DIR)/aarch64-linux-user/qemu-aarch64 \
		$(TMPMOUNTPOINT)/usr/bin/
# change hostname
	-sudo sed -i "s/01/$(shell printf "%02d" $$(($* + 1)))/" \
		$(TMPMOUNTPOINT)/etc/hostname
# umount the image
	-sudo rm -f $(TMPMOUNTPOINT)/usr/bin/qemu-aarch64
	-sudo umount $(TMPMOUNTPOINT)
	-sudo $(QEMU-NBD) --disconnect $(NBD)
endif


CHROOTS := $(addprefix debug-chroot-,$(IDS))
.PHONY: $(CHROOTS)

CHROOT_PROMPT = $(shell cat $(TMPMOUNTPOINT)/etc/hostname)
CHROOT_PROMPT = CHROOT

# chroot into a qcow2 image for qemu
$(CHROOTS): debug-chroot-%: $(IMAGES)/$(MASTERROOTFS_NAME)-snap-%.qcow2
	-mkdir -p $(TMPMOUNTPOINT)
	sudo $(QEMU-NBD) -f qcow2 \
		--connect=$(NBD) \
		$(IMAGES)/$(MASTERROOTFS_NAME)-snap-$*.qcow2
	sudo mount -t auto -o defaults,rw $(NBD) $(TMPMOUNTPOINT)
	-@sudo cp $(QEMU-STATIC) $(TMPMOUNTPOINT)/usr/bin
	-@echo "Entering QEmu CHROOT on $(MASTERROOTFS_NAME)-snap-$*.qcow2"
	-@echo "Type 'exit' to terminate"
ifeq ($(FS),seco)
	-@sudo debian_chroot="$(CHROOT_PROMPT)" chroot $(TMPMOUNTPOINT)
else
	-@sudo debian_chroot="$(CHROOT_PROMPT)" chroot $(TMPMOUNTPOINT) /bin/sh
endif
	-@echo "Leaving CHROOT"
	-@sudo rm -f $(TMPMOUNTPOINT)/usr/bin/qemu-aarch64
	sudo umount $(TMPMOUNTPOINT)
	sudo $(QEMU-NBD) --disconnect $(NBD)

.PHONY: debug-chroot-umount

debug-chroot-umount:
	-sudo umount $(TMPMOUNTPOINT)
	-sudo $(QEMU-NBD) --disconnect $(NBD)

start: run

run: clean-log stop switch $(RUNS)

netup:
	@./setup_network_on_host.sh up

netdown:
	@./setup_network_on_host.sh down

setupnet:
	@./setup_network_on_nodes.sh

$(RUNS): run%: $(UBOOT_FILE) \
		$(IMAGES)/$(MASTERROOTFS_NAME)-snap-%.qcow2 \
		$(QEMU_DTS_DIR)/axiom-board.dtb \
		$(QEMU_DTS_DIR)/axiom-board-long-enqueue.dtb
	@echo "starting qemu[$*]: rootfs: $(MASTERROOTFS_NAME)-snap-$*.qcow2 param: $(QEMU_PARAM)" | tee -a $(LOG_FILE)
	@./start_qemu.sh --id $* --qemu ${QEMU} $(QEMU_PARAM) >> $(LOG_FILE) 2>&1 &

stop:
	@echo "stopping qemu and axiom_switch" | tee -a $(LOG_FILE)
	-@killall $(QEMU) 2> /dev/null
	-@killall $(SWITCH) 2> /dev/null

switch:
	@echo "starting switch: $(SWITCH) $(SWITCH_PARAM)" | tee -a $(LOG_FILE)
	@$(SWITCH_RUN)

log:
	@tail -f -n +1 $(LOG_FILE)

SYNC_PASSWD := root
SYNC_USER := root

$(SYNCS): sync%:
	@echo "SYNC node $$(($*+1))"
	-ssh-keygen -q -f "$(HOME)/.ssh/known_hosts" -R [127.0.0.1]:2220$*
	sshpass -p $(SYNC_PASSWD) rsync -qv -rpD -e "ssh -q -o StrictHostKeyChecking=no -p 2220$* -l $(SYNC_USER)" \
		$$(find $(TARGET_DIR)/* -maxdepth 0 -type d) \
		127.0.0.1:/

sync: $(SYNCS)

help::
	@echo "RUNNING"
	@echo "  run|start        Start nodes and axiom switch; with the following parameters:"
	@echo "    VMS=?             number of node to run (default: $(VMS))"
	@echo "    NETWORK=0|1       enable TCP/IP network between nodes (default: 0)"
	@echo "    LONG_ENQUEUE=0|1  ensure a strict FIFO policy on axiom queue (default: 1)"
	@echo "	   P=0|1             run a 'petalinux' qemu machine, not a builroot machine (default: 0)"
	@echo "    E=0|1             if P=1 run in emulated mode i.e. use the buildroot kernel/dtb (default: 1)"
	@echo "    S=0|1             use a temp snapshot rootfs (default: 1)"
	@echo "  stop             Stop nodes and axiom_switch"
	@echo "  netup            Setup network on host (must run by root prior to run vms)"
	@echo "  netdown          Clean network on host (can be run by root after shutdown vms)"
	@echo "  setupnet         Setup networks and axiom-init on nodes (nodes must be running)"
	@echo "  sync             Copy target root filesystem on running vms"
	@echo " "
	@echo "  run?             Run the ? virtual nodes"
	@echo "  switch           Run the switch"
	@echo "  log              Show qemu log"
	@echo "  debug-chroot-?   Run a chroot into the ?-th qemu rootfs image"
	@echo " "

#
# ARCHIVE
#

.PHONY: archive-vms archive-toolchain archive release

ALL_BR_ROOTFS := $(addsuffix .qcow2,$(addprefix $(IMAGES)/$(MASTERROOTFS_BR_NAME)-snap-,$(IDS)))
ALL_SECO_ROOTFS := $(addsuffix .qcow2,$(addprefix $(IMAGES)/$(MASTERROOTFS_SECO_NAME)-snap-,$(IDS)))

archive-vms: clean-archive-vms ${IMAGES}/u-boot.sd.elf ${IMAGES}/u-boot.elf
	@if [ $(VMS) -lt 4 ]; then echo -e $(_ccred)Warning: VMS \< 4! The archive will contain only rootfs for $(VMS) machines \(used VMS=? to chaneg this\).$(_ccend); fi
	@make P=0 $(ALL_BR_ROOTFS)
	@make P=1 $(ALL_SECO_ROOTFS)
	mkdir -p $(ARCHIVE_VMS_DIR)/images
	mkdir -p $(ARCHIVE_VMS_DIR)/extrae
	cp $(QEMU) $(ARCHIVE_VMS_DIR)/
	cp $(SWITCH) $(ARCHIVE_VMS_DIR)/
	cp $(IMAGES)/*dtb $(ARCHIVE_VMS_DIR)/images/
	cp $(IMAGES)/*elf $(ARCHIVE_VMS_DIR)/images/
	cp $(IMAGES)/*Image $(ARCHIVE_VMS_DIR)/images/
	cp $(IMAGES)/zynq-rootfs*qcow2 $(ARCHIVE_VMS_DIR)/images/
	cd $(ARCHIVE_VMS_DIR)/images ;\
		for f in *snap*.qcow2; do \
		$(QEMU-IMG) rebase -u -b $$(echo $$f | sed 's/-snap-[0-9]*//') $$f;\
		done;
	cp $(QEMU_DTS_DIR)/*dtb $(ARCHIVE_VMS_DIR)/images/
	cp $(ARM_FIRMWARE_DIR)/build/zynqmp/release/bl31/bl31.elf $(ARCHIVE_VMS_DIR)/images/
	cp $(AXIOMHOME)/CHANGELOG $(ARCHIVE_VMS_DIR)/CHANGELOG
	cp -r ./vms.archive/* $(ARCHIVE_VMS_DIR)/
	cp $(TESTS_DIR)/utils/host/extrae_merge.sh $(ARCHIVE_VMS_DIR)/extrae
	cp $(TESTS_DIR)/utils/host/mpi2prv $(ARCHIVE_VMS_DIR)/extrae
	cp -r $(TESTS_DIR)/utils/host/paraver_cfg $(ARCHIVE_VMS_DIR)/extrae
	cp $(OMPSS_TESTS_DIR)/README $(ARCHIVE_VMS_DIR)/README.tests.ompss
	cp $(EVI_APPS_DIR)/tests/README $(ARCHIVE_VMS_DIR)/README.tests.axiom
	cp $(GASNET_TESTS_DIR)/README $(ARCHIVE_VMS_DIR)/README.tests.gasnet
	tar -zcvf $(ARCHIVE_VMS_TGZ).tar.gz $(ARCHIVE_VMS_DIR) --transform s/$(ARCHIVE_VMS_DIR)/$(ARCHIVE_VMS_TGZ)/
	mkdir -p $(ARCHIVE_DIR)
	mv $(ARCHIVE_VMS_TGZ).tar.gz $(ARCHIVE_DIR)


archive-toolchain: clean-archive-toolchain clean-tests
	mkdir -p $(ARCHIVE_TOOLCHAIN_DIR)/toolchain
	mkdir -p $(ARCHIVE_TOOLCHAIN_DIR)/examples/src
	cp -r ./toolchain.archive/* $(ARCHIVE_TOOLCHAIN_DIR)/
	cp -r $(TESTS_DIR)/utils $(ARCHIVE_TOOLCHAIN_DIR)/examples
	cp -r $(TESTS_DIR)/ompss/N-body $(ARCHIVE_TOOLCHAIN_DIR)/examples
	cp -r $(OMPSS_TESTS_DIR)/src/*.c $(ARCHIVE_TOOLCHAIN_DIR)/examples/src
	cp -r $(OMPSS_TESTS_DIR)/src/*.h $(ARCHIVE_TOOLCHAIN_DIR)/examples/src
	cp -r $(OUTPUT_DIR)/host $(ARCHIVE_TOOLCHAIN_DIR)/toolchain/host
	cd $(ARCHIVE_TOOLCHAIN_DIR)/toolchain && ln -s host/usr/aarch64-buildroot-linux-gnu/sysroot sysroot
	cd $(ARCHIVE_TOOLCHAIN_DIR)/toolchain/host/usr/share/mcxx/config.d && \
		sed -i s,$(OUTPUT_DIR)/host,\__AXIOM_TOOLCHAIN__,g *
	tar -zcf $(ARCHIVE_TOOLCHAIN_TGZ).tgz $(ARCHIVE_TOOLCHAIN_DIR) --transform s/$(ARCHIVE_TOOLCHAIN_DIR)/$(ARCHIVE_TOOLCHAIN_TGZ)/
	mkdir -p $(ARCHIVE_DIR)
	mv $(ARCHIVE_TOOLCHAIN_TGZ).tgz $(ARCHIVE_DIR)

archive: archive-vms archive-toolchain

release: clean all archive

help::
	@echo "ARCHIVING"
	@echo "  archive-vms        Build an archive containing VMS"
	@echo "  archive-toolchain  Build an archive containing the toolchain"
	@echo "  archive            Build an archive containing VMS and toolchain"
	@echo "  release            As 'archive' for a point release"
	@echo " "

#
# CLEAN
#

.PHONY: clean-log clean-evi clean-archive-vms clean-archive-toolchain clean distclean-evi distclean clean-tests-axiom clean-tests clean-tests-gasnet clean-tests-ompss

clean-log:
	-@rm $(LOG_FILE)

clean-evi distclean-evi:
	for DIR in $(EVI_NIC_DIR) $(EVI_APPS_DIR) $(QEMU_DTS_DIR) $(EVI_ALLOC_LIB_DIR) $(ALLOCATOR_DIR); do \
		$(MAKE) -C $${DIR} $(subst -evi,,$@) || true;\
	done
	$(MAKE) -C $(EVI_ALLOC_DRV_DIR) CCARCH=$(CCARCH) $(subst -evi,,$@) || true

clean-archive-vms:
	-@rm -rf $(ARCHIVE_VMS_DIR)

clean-archive-toolchain:
	-@rm -rf $(ARCHIVE_TOOLCHAIN_DIR)

clean: clean-log clean-evi clean-archive-vms clean-archive-toolchain clean-tests

distclean: clean-log distclean-evi clean-archive-vms clean-archive-toolchain
#	[ -d "$(TARGET_DIR)" -a -n "$(TARGET_DIR)" ] && rm -rf $(TARGET_DIR) || true

clean-toolchain: distclean
	[ -d "$(OUTPUT_DIR)" -a -n "$(OUTPUT_DIR)" ] && rm -rf $(OUTPUT_DIR) || true

help::
	@echo "CLEANING"
	@echo "  clean                    Clean subprojects"
	@echo "  distclean                Clean subprojects including configuration AND PARTIAL TARGET ROOT FILESYSTEM"
	@echo "  clean-toolchain          As 'distclean' including BUILDROOT TOOLCHAIN"
	@echo " "
	@echo "  clean-log                Clean qemu log file"
	@echo "  clean-evi                Clean subproject (libs,nic,apps,allocator)"
	@echo "  distclean-evi            As 'clean-evi', including configuration"
	@echo "  clean-archive-vms        Clean archive VMs directory"
	@echo "  clean-archive-toolchain  Clean archive toolchain directory"
	@echo " "
	@echo "  clean-tests-axiom        Clean AXIOM API tests"
	@echo "  clean-tests-gasnet       Clean GASNet tests"
	@echo "  clean-tests-ompss        Clean OmpSS tests"
	@echo " "
	@echo "  clean tests              Clean all tests"
	@echo " "

#
# TEST
#

.PHONY: tests-axiom tests tests-gasnet tests-ompss

tests-axiom:
	@$(MAKE) -C $(AXIOM_TESTS_DIR) install

clean-tests-axiom:
	@$(MAKE) -C $(AXIOM_TESTS_DIR) clean

tests-ompss:
	@$(MAKE) -C $(OMPSS_TESTS_DIR) install

clean-tests-ompss:
	@$(MAKE) -C $(OMPSS_TESTS_DIR) clean

tests-gasnet: gasnet
	@$(MAKE) -C $(GASNET_TESTS_DIR) install

clean-tests-gasnet:
	@$(MAKE) -C $(GASNET_TESTS_DIR) clean

tests: tests-axiom tests-gasnet tests-ompss

clean-tests: clean-tests-axiom clean-tests-gasnet clean-tests-ompss

help::
	@echo "TEST"
	@echo "  tests-axiom        Build (and install) AXIOM API tests"
	@echo "  tests-gasnet       Build (and install) GASNet tests"
	@echo "  tests-ompss        Build (and install) OmpSS tests"
	@echo "  tests              Build all tests"
	@echo "  clean-tests-gasnet Clean gasnet tests"
	@echo "  clean-tests-ompss  Clean OMPSS tests"
	@echo "  clean-tests-axiom  Clean AXIOM tests"
	@echo "  clean-test         Clean all tests"
	@echo " "
	@echo "To run test automatically go to ./tests/ directory and read the README. "
	@echo " "

#
# DEB
#

#ifneq ($(P),1)
#
#.PHONY: deb clean-deb distclean-deb
#
#deb clean-deb distclean-deb:
#	$(error deb, clean-deb and distclean-deb makefile target need P=1 on commandline)
#
#else

.PHONY: deb-prepare deb-unprepare deb-install deb clean-deb distclean-deb

ROOTFSTMP:=$(AXIOMHOME)/rootfs-tmp
ROOTFSWRK:=$(AXIOMHOME)/rootfs-wrk

DEBENV := ROOTFS=$(ROOTFSTMP) GCC_TARGET=arm64 CC=$(LINARO)/host/usr/bin/aarch64-linux-gnu-gcc

deb-prepare: qemu
	mkdir -p $(AXIOMHOME)/debs
	-sudo umount $(ROOTFSTMP)
	mkdir -p $(ROOTFSTMP)
	mkdir -p $(ROOTFSWRK)
	sudo mount -t overlay overlay -olowerdir=$(ROOTFS),upperdir=$(ROOTFSTMP),workdir=$(ROOTFSWRK) $(ROOTFSTMP)
	cp $(QEMU_STATIC_BUILD_DIR)/aarch64-linux-user/qemu-aarch64 $(ROOTFSTMP)/usr/bin

deb-unprepare:
	-sudo umount $(ROOTFSTMP)
	sudo rm -r -f $(ROOTFSTMP) $(ROOTFSWRK)

deb-install:
	cp $(AXIOMHOME)/*deb $(ROOTFSTMP)
	sudo chroot $(ROOTFSTMP) /bin/bash -c "/usr/bin/dpkg --install /*.deb"
	cp $(AXIOMHOME)/*deb $(AXIOMHOME)/debs
	rm -f $(ROOTFSTMP)/*.deb
	rm -f $(AXIOMHOME)/*.deb


deb: deb-prepare
# kernel modules
	$(DEBENV) fakeroot $(MAKE) -C $(LOCFILE_DIR) -f debian/rules binary
	mv -f $(AXIOMHOME)/axiomeu-dev_*deb $(AXIOMHOME)/axiomeu_*deb $(AXIOMHOME)/debs
	$(MAKE) deb-install
# extrae
	$(MAKE) extrae
	$(DEBENV) fakeroot $(MAKE) -C $(EXTRAE_DIR) -f debian/rules binary
	$(MAKE) deb-install
# axiom user library & kernel module
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_NIC_DIR) -f debian/rules binary
	$(MAKE) deb-install
# axiom allocator kernel module
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_ALLOC_DRV_DIR) -f debian/rules binary
	$(MAKE) deb-install
# axiom allocator library
	$(MAKE) allocator-lib
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_ALLOC_LIB_DIR) -f debian/rules binary
	$(MAKE) deb-install
# axiom init library
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_APPS_DIR)/axiom-init -f debian/rules binary
	mv $(EVI_APPS_DIR)/*.deb $(AXIOMHOME)
	$(MAKE) deb-install
# axiom run library
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_APPS_DIR)/axiom-run -f debian/rules binary
	mv $(EVI_APPS_DIR)/*.deb $(AXIOMHOME)
	$(MAKE) deb-install
# memory allocator
	$(MAKE) allocator
	$(DEBENV) fakeroot $(MAKE) -C $(ALLOCATOR_DIR) -f debian/rules binary
	$(MAKE) deb-install
# axiom apps
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_APPS_DIR) -f debian/rules binary
	$(MAKE) deb-install
# gasnet
	$(MAKE) gasnet
	$(DEBENV) fakeroot $(MAKE) -C $(GASNET_DIR) -f debian/rules binary
	$(MAKE) deb-install
# nanox
	$(MAKE) nanox
	$(DEBENV) fakeroot $(MAKE) -C $(NANOX_DIR) -f debian/rules binary
	$(MAKE) deb-install

clean-deb: deb-destroy
	$(DEBENV) fakeroot $(MAKE) -C $(EXTRAE_DIR) -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_NIC_DIR) -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_ALLOC_DRV_DIR) -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_ALLOC_LIB_DIR) -f debian/rules  clean
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_APPS_DIR)/axiom-init -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_APPS_DIR)/axiom-run -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(ALLOCATOR_DIR) -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(EVI_APPS_DIR) -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(GASNET_DIR) -f debian/rules clean
	$(DEBENV) fakeroot $(MAKE) -C $(NANOX_DIR) -f debian/rules clean

distclean-deb: clean-deb distclean-extrae distclean-gasnet distclean-nanox
	$(MAKE) -C $(EVI_NIC_DIR) distclean
	$(MAKE) -C $(EVI_ALLOC_DRV_DIR) distclean
	$(MAKE) -C $(EVI_ALLOC_LIB_DIR) distclean
	$(MAKE) -C $(ALLOCATOR_DIR) distclean
	$(MAKE) -C $(EVI_APPS_DIR) distclean

#endif

help::
	@echo "DEBIANization"
	@echo "  deb            Build debian packages"
	@echo " "
	@echo "  clean-deb      Clean all deb build"
	@echo "  distclean-deb  Clean all deb build and compilation"
	@echo " "
	@echo "This targets need P=1 on command line."
