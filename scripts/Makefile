
DATE := $(shell date +%Y%m%d_%H%M%S)-
MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

# number of Virtual Machines to run
VMS := 2

GIT_DESC := $(shell git describe --tags | sed s/axiom-//g)
ARCHIVE_VMS_TGZ := $(DATE)axiom-nic-evisim-$(GIT_DESC)
release: ARCHIVE_VMS_TGZ := axiom-nic-evisim-$(GIT_DESC)
ARCHIVE_TOOLCHAIN_TGZ := $(DATE)axiom-evi-toolchain-$(GIT_DESC)
release: ARCHIVE_TOOLCHAIN_TGZ := axiom-evi-toolchain-$(GIT_DESC)

#
# directories
#(i.e. where files are located)
#
QEMU_DIR := $(MKFILE_DIR)/../axiom-evi-qemu
QEMU_PARAM := ""
BUILDROOT_DIR := $(MKFILE_DIR)/../axiom-evi-buildroot
ARM_FIRMWARE_DIR := $(MKFILE_DIR)/../arm-trusted-firmware
QEMU_DTS_DIR := $(MKFILE_DIR)/./buildroot/qemu-dts
EVI_NIC_DIR := $(MKFILE_DIR)/../axiom-evi-nic
EVI_APPS_DIR := $(MKFILE_DIR)/../axiom-evi-apps
GASNET_DIR := $(MKFILE_DIR)/../axiom-evi-gasnet
GASNET_BUILD_DIR_DEBUG := $(MKFILE_DIR)/../axiom-evi-gasnet/build_debug
GASNET_BUILD_DIR_PERF := $(MKFILE_DIR)/../axiom-evi-gasnet/build_perf
MCXX_DIR := $(MKFILE_DIR)/../axiom-evi-mcxx
MCXX_BUILD_DIR := $(MCXX_DIR)/build
NANOX_DIR := $(MKFILE_DIR)/../axiom-evi-nanox
NANOX_BUILD_DIR_DEBUG := $(NANOX_DIR)/build_debug
NANOX_BUILD_DIR_PERF := $(NANOX_DIR)/build_perf
EVI_ALLOC_DRV_DIR := $(MKFILE_DIR)/../axiom-evi-allocator-drv
EVI_ALLOC_LIB_DIR := $(MKFILE_DIR)/../axiom-evi-allocator-lib
ALLOCATOR_DIR := $(MKFILE_DIR)/../axiom-allocator
OVERLAY_DIR := $(MKFILE_DIR)/buildroot/overlay
OUTPUT_DIR = $(realpath $(MKFILE_DIR)/../output)
#HOST_DIR=$(OUTPUT_DIR)/host
#TARGET_DIR=$(OUTPUT_DIR)/target
TARGET_DIR := $(realpath ${OUTPUT_DIR}/target)
SYSROOT_DIR := $(realpath ${OUTPUT_DIR}/staging)
HOST_DIR := $(realpath ${OUTPUT_DIR}/host)

AXIOM_DTB := axiom-board.dtb
DFLAGS := -g
debug: DFLAGS := -g -DPDEBUG

export DFLAGS

#CCARCH := arm
CCARCH := aarch64
IMAGES := $(OUTPUT_DIR)/images
QEMU := $(QEMU_DIR)/aarch64-softmmu/qemu-system-aarch64
CCPREFIX := ${OUTPUT_DIR}/host/usr/bin/$(CCARCH)-linux-
SWITCH :=$(EVI_NIC_DIR)/axiom_switch/axiom_switch
SWITCH_PARAM := -r -n $(VMS)
ARCHIVE_VMS_DIR := archive_vms
ARCHIVE_TOOLCHAIN_DIR := archive_toolchain
TESTS_DIR := $(MKFILE_DIR)/../tests/
OMPSS_TESTS_DIR := $(TESTS_DIR)/ompss
ARCHIVE_DIR := archive
LOG_FILE := make.log

BDPATH := $(OUTPUT_DIR)/host/usr/bin/:$(PATH)

ifndef AXIOMHOME
    AXIOMHOME=$(realpath ${MKFILE_DIR}/..)
endif

ifdef SWITCH
SWITCH_RUN = $(SWITCH) $(SWITCH_PARAM) >> $(LOG_FILE) 2>&1 &
else
SWITCH_RUN=
SWITCH_PARAM=
endif

IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))
SYNCS := $(addprefix sync,$(IDS))

ifeq ($(LONG_ENQUEUE), 1)
QEMU_PARAM = "-l"
endif

BUILDCPUS:=$(shell getconf _NPROCESSORS_ONLN)

#
# MAIN targets
#

.PHONY: all debug target help

all: buildroot arm-firmware qemu

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
	cd $(QEMU_DIR) && ./axiom_configure.sh

config-alloc: config-buildroot
	cd $(EVI_ALLOC_LIB_DIR) &&  autoreconf -fi
	cd $(ALLOCATOR_DIR) && autoreconf -fi

config-buildroot:
	cd $(BUILDROOT_DIR) && $(MAKE) O=$(OUTPUT_DIR) evi_ultrascale_defconfig && $(MAKE) O=$(OUTPUT_DIR)

help::
	@echo "CONFIGURE"
	@echo "  configure          Configure the subrojects (and build toolschain)"
	@echo "  "
	@echo "  config-alloc       Configure memory allocator"
	@echo "  config-buildroot   Build buildroot toolschain"
	@echo " "

#
# BUILD target filesystem
#

.PHONY: libs allocator-driver allocator-libs allocator axiom-nic axiom-apps _buildroot buildroot arm-firmware qemu-dts qemu

libs:
	$(MAKE) -C $(EVI_NIC_DIR) libs
	$(MAKE) -C $(EVI_APPS_DIR) libs

allocator-driver: libs
	$(MAKE) -C $(EVI_ALLOC_DRV_DIR)
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(EVI_ALLOC_DRV_DIR) install
	$(MAKE) DESTDIR=$(SYSROOT_DIR) -C $(EVI_ALLOC_DRV_DIR) install

allocator-lib: libs
	cd $(EVI_ALLOC_LIB_DIR) && CC=$(CCPREFIX)gcc ./configure \
		--host=aarch64-buildroot-linux-gnu --prefix=/usr --disable-debug --enable-extld
	$(MAKE) -C $(EVI_ALLOC_LIB_DIR)
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(EVI_ALLOC_LIB_DIR) install
	$(MAKE) DESTDIR=$(SYSROOT_DIR) -C $(EVI_ALLOC_LIB_DIR) install
# libtool fix :-(
	for LIB in $(SYSROOT_DIR)/usr/lib/libevi_lmm.la $(TARGET_DIR)/usr/lib/libevi_lmm.la; do \
		sed -i -e "s,libdir='/usr/lib',libdir='$(SYSROOT_DIR)/usr/lib'," \
		       -e "s, /usr/lib, $(SYSROOT_DIR)/usr/lib," \
		       $${LIB} ;\
	done

allocator: allocator-driver allocator-lib
	cd $(ALLOCATOR_DIR) && CC=$(CCPREFIX)gcc ./configure \
		--host=aarch64-buildroot-linux-gnu --prefix=/usr
	$(MAKE) -C $(ALLOCATOR_DIR)
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(ALLOCATOR_DIR) install
	$(MAKE) DESTDIR=$(SYSROOT_DIR) -C $(ALLOCATOR_DIR) install
# libtools fix :-(
	for LIB in $(SYSROOT_DIR)/usr/lib/libaxiom_allocator*.la; do \
		sed -i -e "s,libdir='/usr/lib',libdir='$(SYSROOT_DIR)/usr/lib'," \
		       -e "s, /usr/lib, $(SYSROOT_DIR)/usr/lib," \
		       $${LIB} ;\
	done

axiom-nic: libs allocator
	$(MAKE) -C $(EVI_NIC_DIR) install

axiom-apps: libs allocator
	$(MAKE) -C $(EVI_APPS_DIR) install

_buildroot:
	$(MAKE) -C $(OUTPUT_DIR)

buildroot: axiom-apps axiom-nic
	$(MAKE) _buildroot

arm-firmware:
	$(MAKE) -C $(ARM_FIRMWARE_DIR) CROSS_COMPILE=$(CCPREFIX) RESET_TO_BL31=1 PLAT=zynqmp all fip

qemu-dts:
	$(MAKE) -C $(QEMU_DTS_DIR) DTC=$(HOST_DIR)/usr/bin/linux-dtc

qemu: qemu-dts
	$(MAKE) -C $(QEMU_DIR) -j$(BUILDCPUS)

help::
	@echo "BUILD"
	@echo "  libs               Build axiom API libraries"
	@echo "  allocator-driver   Build Evidence kernel allocator driver"
	@echo "  allocator-lib      Build Evidence memory allocator libreries"    
	@echo "  allocator          Build memory allocator"
	@echo "  axiom-nic          Build axiom kernel driver (and, eventually, user API libs)"
	@echo "  axiom-app          Build axiom user applications (and, eventually, run/init API libs)"
	@echo "  buildroot          Build target filesystem"
	@echo "  arm-firmware       Build arm firmware"
	@echo "  qemu-dts           Build system DTS"
	@echo "  qemu               Build QEMU"
	@echo " "
	@echo "  _buildroot        Build ONLY buildroot target filesystem"
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
		--enable-debug ;\
	fi

gasnet-debug: gasnet-config-debug
	@$(MAKE) -C $(GASNET_BUILD_DIR_DEBUG)
	@$(MAKE) -C $(GASNET_BUILD_DIR_DEBUG) DESTDIR=$(SYSROOT_DIR) install

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
	@$(MAKE) -C $(GASNET_BUILD_DIR_PERF) DESTDIR=$(SYSROOT_DIR) install

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
	@echo "  gasnet-clean         Clean gasnet libraries"
	@echo "  gasnet-distclean     Clean gasnet libraries including configuration"
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

.PHONY: nanox-bootstrap nanox-debug-config nanox-debug nanox-perf-config nanox-perf nanox clean-nanox distclean-nanox

nanox-bootstrap:
	@if [ ! -r $(NANOX_DIR)/configure ]; then \
		cd $(NANOX_DIR);\
		./bootstrap;\
	fi

nanox-config-debug: nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_DEBUG)
	@cd $(NANOX_BUILD_DIR_DEBUG); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr \
		../scripts/cross-configure-axiom_debug.sh

nanox-debug: nanox-config-debug
	@$(MAKE) -j$(BUILDCPUS) -C $(NANOX_BUILD_DIR_DEBUG)
	@$(MAKE) -C $(NANOX_BUILD_DIR_DEBUG) DESTDIR=$(SYSROOT_DIR) install
	@$(MAKE) -C $(NANOX_BUILD_DIR_DEBUG) DESTDIR=$(TARGET_DIR) install

nanox-config-perf: nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_PERF)
	@cd $(NANOX_BUILD_DIR_PERF); AXIOMHOME=$(AXIOMHOME) PREFIX=/usr \
		../scripts/cross-configure-axiom_performance.sh

nanox-perf: nanox-config-perf
	@$(MAKE) -j$(BUILDCPUS) -C $(NANOX_BUILD_DIR_PERF)
	@$(MAKE) -C $(NANOX_BUILD_DIR_PERF) DESTDIR=$(SYSROOT_DIR) install
	@$(MAKE) -C $(NANOX_BUILD_DIR_PERF) DESTDIR=$(TARGET_DIR) install

nanox: nanox-debug nanox-perf

clean-nanox:
	@[ -d $(NANOX_BUILD_DIR_DEBUG) ] && make -C $(NANOX_BUILD_DIR_DEBUG) clean
	@[ -d $(NANOX_BUILD_DIR_PERF) ] && make -C $(NANOX_BUILD_DIR_PERF) clean

distclean-nanox:
	@[ -d $(NANOX_BUILD_DIR_DEBUG) ] && rm -rf $(NANOX_BUILD_DIR_DEBUG) || true
	@[ -d $(NANOX_BUILD_DIR_PERF) ] && rm -rf $(NANOX_BUILD_DIR_PERF) || true

help::
	@echo "NANOX (OMPSS)"
	@echo "  nanox               Build Nanos++ libraries"
	@echo "  clean-nanox         Clean Nanos++ libraries"
	@echo "  distclean-nanox     Clean Nanos++ libraries including configuration"
	@echo " "
	@echo "  nanox-bootstrap     Configuration bootsrap"
	@echo "  nanox-config-debug  Configure debug library"
	@echo "  nanox-config-perf   Configure performace library"
	@echo "  nanox-debug         Build debug library"
	@echo "  nanox-perf          Build performance library"
	@echo " "

#
# MCXX
#

.PHONY: mcxx-boostrap mcxx-config mcxx clean-mcxx distclean-mcxx

mcxx-bootstrap:
	@if [ ! -r $(MCXX_DIR)/configure ]; then \
		cd $(MCXX_DIR);\
		autoreconf -fiv;\
	fi

mcxx-config: nanox mcxx-bootstrap
	@mkdir -p $(MCXX_BUILD_DIR)
	cd $(MCXX_BUILD_DIR); AXIOMHOME=$(AXIOMHOME) PREFIX=$(HOST_DIR)/usr \
		../scripts/cross-configure-axiom.sh

mcxx: mcxx-config
	@make -j$(BUILDCPUS) PATH=$(PATH):$(HOST_DIR)/usr/bin -C $(MCXX_BUILD_DIR)
#	@make -C $(MCXX_BUILD_DIR) DESTDIR=$(HOST_DIR) install
	@make -C $(MCXX_BUILD_DIR) install
	@cp $(HOST_DIR)/usr/lib/pkgconfig/mcxx.pc $(SYSROOT_DIR)/usr/lib/pkgconfig/mcxx.pc

clean-mcxx:
	@[ -d $(MCXX_BUILD_DIR) ] && make -C $(MCXX_BUILD_DIR) clean

distclean-mcxx:
	@[ -d $(MCXX_BUILD_DIR) ] && rm -rf $(MCXX_BUILD_DIR) || true

help::
	@echo "MCXX (OMPSS)"
	@echo "  mcxx              Build Mercurium source cross compiler"
	@echo "  clean-mcxx        Clean Mercurium"
	@echo "  distclean-mcxx    Clean Mercurium includind configuration"
	@echo ""
	@echo "  mcxx-bootstrap    Configuration bootstrap"
	@echo "  mcxx-config       Mercurium configuration"
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
	@echo ""

#
# ARCHIVE
#

.PHONY: archive release archive-toolchain

archive: clean-archive-vms
	mkdir -p $(ARCHIVE_VMS_DIR)/images
	cp $(QEMU) $(ARCHIVE_VMS_DIR)/
	cp $(SWITCH) $(ARCHIVE_VMS_DIR)/
	cp $(IMAGES)/*dtb $(ARCHIVE_VMS_DIR)/images/
	cp $(IMAGES)/*elf $(ARCHIVE_VMS_DIR)/images/
	cp $(IMAGES)/*Image $(ARCHIVE_VMS_DIR)/images/
	cp $(QEMU_DTS_DIR)/$(AXIOM_DTB) $(ARCHIVE_VMS_DIR)/images/
	cp $(ARM_FIRMWARE_DIR)/build/zynqmp/release/bl31/bl31.elf $(ARCHIVE_VMS_DIR)/images/
	cp $(MKFILE_DIR)/../CHANGELOG $(ARCHIVE_VMS_DIR)/CHANGELOG
	cp -r $(MKFILE_DIR)/vms.archive/* $(ARCHIVE_VMS_DIR)/
	tar -zcvf $(ARCHIVE_VMS_TGZ).tgz $(ARCHIVE_VMS_DIR) --transform s/$(ARCHIVE_VMS_DIR)/$(ARCHIVE_VMS_TGZ)/
	mkdir -p $(ARCHIVE_DIR)
	mv $(ARCHIVE_VMS_TGZ).tgz $(ARCHIVE_DIR)


archive-toolchain: clean-archive-toolchain
	mkdir -p $(ARCHIVE_TOOLCHAIN_DIR)/toolchain
	mkdir -p $(ARCHIVE_TOOLCHAIN_DIR)/examples
	cp -r $(MKFILE_DIR)/toolchain.archive/* $(ARCHIVE_TOOLCHAIN_DIR)/
	cp -r $(TESTS_DIR)/utils $(ARCHIVE_TOOLCHAIN_DIR)/examples
	cp -r $(OMPSS_TESTS_DIR)/src $(ARCHIVE_TOOLCHAIN_DIR)/examples
	cp -r $(OUTPUT_DIR)/host $(ARCHIVE_TOOLCHAIN_DIR)/toolchain/host
	cd $(ARCHIVE_TOOLCHAIN_DIR)/toolchain && ln -s host/usr/aarch64-buildroot-linux-gnu/sysroot sysroot
	cd $(ARCHIVE_TOOLCHAIN_DIR)/toolchain/host/usr/share/mcxx/config.d && \
		sed -i s,$(OUTPUT_DIR)/host,\__AXIOM_TOOLCHAIN__,g *
	tar -zcf $(ARCHIVE_TOOLCHAIN_TGZ).tgz $(ARCHIVE_TOOLCHAIN_DIR) --transform s/$(ARCHIVE_TOOLCHAIN_DIR)/$(ARCHIVE_TOOLCHAIN_TGZ)/
	mkdir -p $(ARCHIVE_DIR)
	mv $(ARCHIVE_TOOLCHAIN_TGZ).tgz $(ARCHIVE_DIR)

release: clean all archive archive-toolchain

help::
	@echo "ARCHIVING"
	@echo "  archive            Build an archive containing all needed runtime"
	@echo "  archive-toolchain  Build an archive containing the toolchain"
	@echo "  release            As 'archive' for a point release"
	@echo " "

#
# RUNNING
#

.PHONY: start run netup netdown setupnet stop switch log $(RUNS) sync $(SYNCS)

start: run

run: clean-log stop switch $(RUNS)

netup:
	@./setup_network_on_host.sh up

netdown:
	@./setup_network_on_host.sh down

setupnet:
	@./setup_network_on_nodes.sh

$(RUNS): run%:
	@echo "starting qemu[$*]: $(QEMU) param: $(QEMU_PARAM)" | tee -a $(LOG_FILE)
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
	@echo "    NETWORK=1         enable TCP/IP network between nodes"
	@echo "    LONG_ENQUEUE=1    ensure a strict FIFO policy on axiom queue"
	@echo "  stop             Stop nodes and axiom_switch"
	@echo "  netup            Setup network on host (must run by root prior to run vms)"
	@echo "  netdown          Clean network on host (can be run by root after shutdown vms)"
	@echo "  setupnet         Setup networks and axiom-init on nodes (nodes must be running)"
	@echo "  sync             Copy target root filesystem on running vms"
	@echo " "
	@echo "  run?             Run the ? virtual nodes"
	@echo "  switch           Run the switch"
	@echo "  log              Show qemu log"
	@echo " "

#
# CLEAN
#

.PHONY: clean-log clean-evi clean-archive-vms clean-archive-toolchain clean distclean-evi distclean

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

clean: clean-log clean-evi clean-archive-vms clean-archive-toolchain

distclean: clean-log distclean-evi clean-archive-vms clean-archive-toolchain
#	[ -d "$(TARGET_DIR)" -a -n "$(TARGET_DIR)" ] && rm -rf $(TARGET_DIR) || true

clean-toolschain: distclean
	[ -d "$(OUTPUT_DIR)" -a -n "$(OUTPUT_DIR)" ] && rm -rf $(OUTPUT_DIR) || true

help::
	@echo "CLEANING"
	@echo "  clean                    Clean subprojects"
	@echo "  distclean                Clean subprojects including configuration AND PARTIAL TARGET ROOT FILESYSTEM"
	@echo "  clean-toolschain         As 'distclean' including BUILDROOT TOOLSCHAIN"
	@echo " "
	@echo "  clean-log                Clean qemu log file"
	@echo "  clean-evi                Clean subproject (libs,nic,apps,allocator)"
	@echo "  distclean-evi            As 'clean-evi', including configuration"
	@echo "  clean-archive-vms        Clean archive VMs directory"
	@echo "  clean-archive-toolchain  Clean archive toolchain directory"
	@echo " "

#
# TEST
#

.PHONY: tests-axiom tests-all tests-gasnet tests-ompss

tests-axiom: axiom-nic axiom-apps
	@$(MAKE) -C $(EVI_APPS_DIR)/tests install

tests-ompss:
	@$(MAKE) -C $(OMPSS_TESTS_DIR) install

tests-gasnet: gasnet
	@$(MAKE) -j$(BUILDCPUS) -C $(GASNET_BUILD_DIR_DEBUG) tests-par
	@$(MAKE) -j$(BUILDCPUS) -C $(GASNET_BUILD_DIR_PERF) tests-par
	for DESTDIR in $(TARGET_DIR); do \
		mkdir -p $${DESTDIR}/root/tests_gasnet_dbg ;\
		cp $(GASNET_BUILD_DIR_DEBUG)/axiom-conduit/test* \
			$${DESTDIR}/root/tests_gasnet_dbg ;\
		mkdir -p $${DESTDIR}/root/tests_gasnet_per ;\
		cp $(GASNET_BUILD_DIR_PERF)/axiom-conduit/test* \
			$${DESTDIR}/root/tests_gasnet_per ;\
	done

tests-all: tests-axiom tests-gasnet tests-ompss

help::
	@echo "TEST"
	@echo "  tests-axiom   Build (and install) AXIOM API tests"
	@echo "  tests-gasnet  Build (and install) GASNet tests"
	@echo "  tests-ompss   Build (and install) OmpSS tests"
	@echo " "
	@echo "  tests-all     Build all tests"
	@echo " "
