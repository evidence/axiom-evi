#
# utility
#

# pretty printing
_ccred:="\033[0;31m"
ccred:=$(shell echo -e $(_ccred))
ccgreen:=$(shell echo -e "\033[0;32m")
ccblue:=$(shell echo -e "\033[0;34m")
_ccend:="\033[0m"
ccend:=$(shell echo -e $(_ccend))

# include subproject locations
include locations.mk
export AXIOM_DIR

# include configuration makefile
include configure.mk

# number of build host cores
BUILDCPUS := $(shell getconf _NPROCESSORS_ONLN)

#
# mafefile variable and custom targets
#

ifeq ($(MODE),aarch64)

#
# aarch64 makefile variables and custom targets
#

ROOTFSWRK:=$(AXIOM_DIR)/rootfs-wrk

prepare-install:
	@if ! findmnt -n $(ROOTFSTMP) >/dev/null; then \
	  mkdir -p $(ROOTFSTMP) ;\
	  mkdir -p $(ROOTFSWRK) ;\
	  sudo mount -t overlay overlay \
	    -olowerdir=$(ROOTFS),upperdir=$(ROOTFSTMP),workdir=$(ROOTFSWRK) \
	    $(ROOTFSTMP) ;\
	fi

prepare-install-force:
	-sudo umount $(ROOTFSTMP)
	$(MAKE) prepare-install

unprepare-install:
	-sudo umount $(ROOTFSTMP)

remove-tempdir: unprepare-install
	sudo rm -r -f $(ROOTFSTMP) $(ROOTFSWRK)

TARGET_DIR:=$(ROOTFSTMP)
LINARO_HOST_DIR:=$(LINARO)/host
LINARO_PATH:=$(LINARO_HOST_DIR)/usr/bin:$(PATH)
LINARO_CC:=$(LINARO)/host/usr/bin/aarch64-linux-gnu-gcc
SYS_COMP_ID=x86_64-linux-gnu
SYS_RUN_ID:=aarch64-linux-gnu
DEFAULT_PREFIX=/usr
ifndef FAKEROOTKEY
FAKEROOT:=fakeroot -i $(ROOTFSTMP).faked -s $(ROOTFSTMP).faked
else
FAKEROOT:=
endif
SUDO:=

else ifeq ($(MODE),x86)

#
# x86 makefile variables and custom targets
#

prepare-install:

prepare-install-force: prepare-install

unprepare-install:

TARGET_DIR:=/
LINARO_HOST_DIR:=
LINARO_PATH:=$(PATH)
LINARO_CC:=$(shell which gcc)
# x86_64-pc-linux-gnu
SYS_COMP_ID:=x86_64-linux-gnu
SYS_RUN_ID:=x86_64-linux-gnu
DEFAULT_PREFIX=/usr/local
FAKEROOT:=
SUDO:=sudo -E

endif

#
# CONFIGURE
#

.PHONY: configure

configure: configure-save
	make -C $(NIC_DIR) configure
	make -C $(APPS_DIR) configure
ifeq ($(DISABLE_INSTR),0)
	cd $(EXTRAE_DIR) && autoreconf -fiv
endif
	cd $(ALLOCATOR_LIB_DIR) &&  autoreconf -fiv
	cd $(ALLOCATOR_DIR) && autoreconf -fiv

#
# MAIN targets
#

.PHONY: all build clean distclean mrproper clean-all help mount-temp umount-temp

.DEFAULT_GOAL := all

build: nic apps

clean: extrae-clean libs-clean allocator-lib-clean allocator-drv-clean allocator-clean apps-clean nic-clean

distclean: configure-clean extrae-distclean libs-distclean allocator-lib-distclean allocator-drv-distclean allocator-distclean apps-distclean nic-distclean

mrproper: extrae-mrproper libs-mrproper allocator-lib-mrproper allocator-drv-mrproper allocator-mrproper apps-mrproper nic-mrproper configure-clean 

clean-all: clean gasnet-clean ompss-clean tests-clean

distclean-all: distclean gasnet-distclean ompss-distclean tests-distclean

mrproper-all: mrproper gasnet-mrproper ompss-mrproper tests-mrproper remove-tempdir deb-remove-tempdir

all: build gasnet ompss tests

mount-temp: prepare-install deb-prepare

umount-temp: unprepare-install deb-unprepare


# help is at the bottom of the makefile

#
# EXTRAE
#

.PHONY: extrae-config-force extrae-config extrae-build extrae
.PHONY: extrae-clean extrae-distclean extrae-mrproper

extrae-config-force:
	@mkdir -p $(EXTRAE_BUILD_DIR)
	cd $(EXTRAE_BUILD_DIR); ../scripts/$(MODE)-configure-axiom.sh

extrae-config:
	@if [ ! -r "$(EXTRAE_BUILD_DIR)/config.status" ]; then \
		$(MAKE) extrae-config-force; \
	fi

extrae-build: extrae-config
# why CFLAGS and LDFLAGS can be not passed during configure phase???
	@PATH=$(LINARO_PATH) $(MAKE) -j$(BUILDCPUS) -C $(EXTRAE_BUILD_DIR) \
	  CFLAGS="--sysroot=$(ROOTFS) \
		-I${ROOTFS}/usr/include/aarch64-linux-gnu \
		-L${ROOTFS}/usr/lib/aarch64-linux-gnu \
		-L${ROOTFS}/lib/aarch64-linux-gnu \
		-fPIE" \
	  LDFLAGS="--sysroot=$(ROOTFS) \
		-L${ROOTFS}/usr/lib/aarch64-linux-gnu \
		-L${ROOTFS}/lib/aarch64-linux-gnu \
		-Wl,-rpath-link -Wl,$(ROOTFS)/usr/lib/aarch64-linux-gnu \
		-Wl,-rpath-link -Wl,$(ROOTFS)/lib/aarch64-linux-gnu \
		--shared"

extrae: extrae-build prepare-install
	@PATH=$(LINARO_PATH) $(MAKE) -C $(EXTRAE_BUILD_DIR) DESTDIR=$(TARGET_DIR) install

extrae-clean:
	[ -d $(EXTRAE_BUILD_DIR) ] && make -C $(EXTRAE_BUILD_DIR) clean || true

extrae-distclean:
	[ -d $(EXTRAE_BUILD_DIR) ] && rm -rf $(EXTRAE_BUILD_DIR) || true

extrae-mrproper: extrae-distclean
	-rm -f $(EXTRAE_DIR)/configure

.PHONY: _extrae
ifeq ($(DISABLE_INSTR),1)
_extrae:
else
_extrae: extrae
endif

help::
	@echo "EXTRAE (OMPSS)"
	@echo "  extrae              Build Extrae library"
	@echo "  extrae-clean        Clean Extrae"
	@echo "  extrae-distclean    Clean Extrae includind configuration"
	@echo "  extrae-mrproper     Clean Extrae includind configuration and autoconfig"
	@echo ""
	@echo "  extrae-config-force Force the configuration of Extrae"
	@echo "  extrae-config       Configure Extrae"
	@echo "  extrae-build        Compile Extrae"
	@echo ""

#
# AXIOM USER LIBRARIES
#

.PHONY: libs-build libs
.PHONY: libs-clean libs-distclean libs-mrproper

libs-build: _extrae prepare-install
	$(MAKE) -C $(NIC_DIR) libs-install
	$(MAKE) -C $(APPS_DIR) libs-install

libs: libs-build
	$(SUDO) $(MAKE) -C $(NIC_DIR) libs-install
	$(SUDO) $(MAKE) -C $(APPS_DIR) libs-install

libs-clean:
	$(MAKE) -C $(NIC_DIR) libs-clean
	$(MAKE) -C $(APPS_DIR) libs-clean

libs-distclean:
	-$(MAKE) -C $(NIC_DIR) libs-distclean
	-$(MAKE) -C $(APPS_DIR) libs-distclean

libs-mrproper:
	-$(MAKE) -C $(NIC_DIR) libs-distclean
	-$(MAKE) -C $(APPS_DIR) libs-distclean

help::
	@echo "AXIOM USER LIBRARIES"
	@echo "  libs              Build and install axiom library"
	@echo "  libs-clean        Clean axiom library"
	@echo "  libs-distclean    Clean axiom library includind configuration"
	@echo "  libs-mrproper     Clean axiom library includind configuration and autoconfig"
	@echo ""
	@echo "  libs-build        Compile Extrae"
	@echo ""

#
#  ALLOCATOR DRV
#

.PHONY: allocator-drv-build allocator-drv
.PHONY: allocator-drv-clean allocator-drv-distclean allocator-drv-mrproper

allocator-drv-build: prepare-install
	$(MAKE) -C $(ALLOCATOR_DRV_DIR)

allocator-drv: allocator-drv-build
	$(SUDO) $(MAKE) -C $(ALLOCATOR_DRV_DIR) DESTDIR=$(TARGET_DIR) install

allocartor-drv-clean:
	$(MAKE) -C $(ALLOCATOR_DRV_DIR) clean

allocator-drv-distclean:
	-$(MAKE) -C $(ALLOCATOR_DRV_DIR) distclean

allocator-drv-mrproper: allocator-drv-distclean

help::
	@echo "AXIOM ALLOCATOR DRIVER"
	@echo "  allocator-drv              Build and install allocator driver"
	@echo "  allocator-drv-clean        Clean allocator driver"
	@echo "  allocator-drv-distclean    Clean allocator driver includind configuration"
	@echo "  allocator-drv-mrproper     Clean allocator driver includind configuration and autoconfig"
	@echo ""
	@echo "  allocator-drv-build        Compile allocator driver"
	@echo ""

#
# ALLOCATOR LIB
#

.PHONY: allocator-lib-config-force allocator-lib-config allocator-lib-build allocator-lib
.PHONY: allocator-lib-clean allocator-lib-distclean allocator-lib-mrproper

allocator-lib-config-force:
	cd $(ALLOCATOR_LIB_DIR) && PATH=$(LINARO_PATH) ./configure \
	  --host=$(SYS_RUN_ID) \
	  --prefix=$(DEFAULT_PREFIX) \
	  --disable-debug \
	  --enable-extld

allocator-lib-config:
	if [ ! -r "$(ALLOCATOR_LIB_DIR)/config.status" ]; then \
	  $(MAKE) allocator-lib-config-force; \
	fi

allocator-lib-build: allocator-lib-config prepare-install
	PATH=$(LINARO_PATH) $(MAKE) -C $(ALLOCATOR_LIB_DIR)

allocator-lib: allocator-lib-build
	$(SUDO) $(MAKE) -C $(ALLOCATOR_LIB_DIR) DESTDIR=$(TARGET_DIR) install
ifeq ($(MODE),x86)
        # missing files :-(
	$(SUDO) mkdir -p $(DEFAULT_PREFIX)/axiom-evi-allocator-lib
	$(SUDO) cp $(ALLOCATOR_LIB_DIR)/ldscript/x86/xs_map64.lds \
	  $(DEFAULT_PREFIX)/axiom-evi-allocator-lib
else
        # libtool fix :-(
	for LIB in $(TARGET_DIR)/$(DEFAULT_PREFIX)/lib/libevi_lmm.la; do \
	  sed -i -e "s,libdir='$(DEFAULT_PREFIX)/lib',libdir='$(TARGET_DIR)$(DEFAULT_PREFIX)/lib'," \
		 -e "s, $(DEFAULT_PREFIX)/lib, $(TARGET_DIR)/$(DEFAULT_PREFIX)/lib," \
		 $${LIB} ;\
	done
endif

allocator-lib-clean:
	[ -f $(ALLOCATOR_LIB_DIR)/Makefile ] && \
		$(MAKE) -C $(ALLOCATOR_LIB_DIR) clean || true

allocator-lib-distclean:
	[ -f $(ALLOCATOR_LIB_DIR)/Makefile ] && \
		$(MAKE) -C $(ALLOCATOR_LIB_DIR) distclean || true

allocator-lib-mrproper: allocator-lib-distclean

help::
	@echo "AXIOM ALLOCATOR LIB"
	@echo "  allocator-lib              Build and install allocator lib"
	@echo "  allocator-lib-clean        Clean allocator lib"
	@echo "  allocator-lib-distclean    Clean allocator lib includind configuration"
	@echo "  allocator-lib-mrproper     Clean allocator lib includind configuration and autoconfig"
	@echo ""
	@echo "  allocator-lib-config-force Configure allocator lib"
	@echo "  allocator-lib-config       Configure allocator lib if not already"
	@echo "  allocator-lib-build        Compile allocator lib"
	@echo ""

#
# ALLOCATOR
#

.PHONY: allocator-config-force allocator-config allocator-build allocator
.PHONY: allocator-clean allocator-distclean allocator-mrproper

ifeq ($(MODE),x86)
ALLOCATOR_CONFIG_OTHER_ARGS:=--with-allocator-lds=allocator_x86.lds
else
ALLOCATOR_CONFIG_OTHER_ARGS:=
endif

allocator-config-force:
	mkdir -p $(ALLOCATOR_BUILD_DIR)
	cd $(ALLOCATOR_BUILD_DIR) && PATH=$(LINARO_PATH) ../configure \
		--host=$(SYS_RUN_ID) \
		--prefix=$(DEFAULT_PREFIX) \
		--with-axiom-sysroot=$(TARGET_DIR) \
		$(ALLOCATOR_CONFIG_OTHER_ARGS)

allocator-config: allocator-lib
	if [ ! -r "$(ALLOCATOR_BUILD_DIR)/config.status" ]; then \
		$(MAKE) allocator-config-force; \
	fi

allocator-build: allocator-config prepare-install
	PATH=$(LINARO_PATH) $(MAKE) -C $(ALLOCATOR_BUILD_DIR)

allocator: allocator-build
	PATH=$(LINARO_PATH) $(SUDO) $(MAKE) -C $(ALLOCATOR_BUILD_DIR) \
	  DESTDIR=$(TARGET_DIR) install
ifeq ($(FS),aarch64)
        # libtools fix :-(
	for LIB in $(TARGET_DIR)$(DEFAULT_PREFIX)/lib/libaxiom_allocator*.la; do \
	  sed -i -e "s,libdir='$(DEFAULT_PREFIX)/lib',libdir='$(TARGET_DIR)$(DEFAULT_PREFIX)/lib'," \
	  -e "s, $(DEFAULT_PREFIX)/lib, $(TARGET_DIR)$(DEFAULT_PREFIX)/lib," \
		       $${LIB} ;\
	done
endif

allocator-clean:
	[ -f $(ALLOCATOR_BUILD_DIR)/Makefile ] && \
		$(MAKE) -C $(ALLOCATOR_BUILD_DIR) clean || true

allocator-distclean:
	[ -d $(ALLOCATOR_BUILD_DIR) ] && \
		rm -r -f $(ALLOCATOR_BUILD_DIR) || true

allocator-mrproper: allocator-distclean

help::
	@echo "AXIOM ALLOCATOR"
	@echo "  allocator              Build and install allocator"
	@echo "  allocator-clean        Clean allocator"
	@echo "  allocator-distclean    Clean allocator includind configuration"
	@echo "  allocator-mrproper     Clean allocator includind configuration and autoconfig"
	@echo ""
	@echo "  allocator-config-force Configure allocator lib"
	@echo "  allocator-config       Configure allocator lib if not already"
	@echo "  allocator-build        Compile allocator lib"
	@echo ""

clean: extrae-clean libs-clean allocator-drv-clean allocator-lib-clean allocator-clean

#
# NIC
#

.PHONY: nic-build nic nic-clean nic-distclean nic-mrproper

NIC_INSTALL_OTHER_ARGS:=

nic-build: _extrae libs allocator-drv prepare-install
	$(MAKE) -C $(NIC_DIR)

nic: nic-build
	$(SUDO) $(MAKE) -C $(NIC_DIR) $(NIC_INSTALL_OTHER_ARGS) install

nic-clean:
	$(MAKE) -C $(NIC_DIR) clean

nic-distclean:
	-$(MAKE) -C $(NIC_DIR) distclean

nic-mrproper: nic-distclean

help::
	@echo "AXIOM NIC DRIVER"
	@echo "  nic              Build and install nic driver"
	@echo "  nic-clean        Clean nic driver"
	@echo "  nic-distclean    Clean nic driver includind configuration"
	@echo "  nic-mrproper     Clean nic driver includind configuration and autoconfig"
	@echo ""
	@echo "  nic-build        Compile allocator lib"
	@echo ""

#
# APPS
#

.PHONY: apps-build apps apps-clean apps-distclean apps-mrproper

apps-build: libs allocator prepare-install
	$(MAKE) -C $(APPS_DIR)

apps: apps-build
	$(SUDO) $(MAKE) -C $(APPS_DIR) install

apps-clean:
	$(MAKE) -C $(APPS_DIR) clean

apps-distclean:
	-$(MAKE) -C $(APPS_DIR) distclean

apps-mrproper:
	-$(MAKE) -C $(APPS_DIR) mrproper

help::
	@echo "AXIOM APPLICATIONS"
	@echo "  apps              Build and install axiom applications"
	@echo "  apps-clean        Clean axiom applications"
	@echo "  apps-distclean    Clean axiom applications driverincludind configuration"
	@echo "  apps-mrproper     Clean axiom applications driverincludind configuration and autoconfig"
	@echo ""
	@echo "  apps-build        Compile axiom applications"
	@echo ""

#
# GASNET
#

ifeq ($(MODE),x86)
GASNET_CONFIG_OTHER_DEBUG_ARGS:=
GASNET_CONFIG_OTHER_PERF_ARGS:=
else
GASNET_CONFIG_OTHER_DEBUG_ARGS:= \
	--with-axiom-host=$(LINARO_HOST_DIR) \
	--prefix=/usr \
	--libdir=/usr/lib/debug \
	--includedir=/usr/include/debug
GASNET_CONFIG_OTHER_PERF_ARGS:= \
	--with-axiom-host=$(LINARO_HOST_DIR) \
	--prefix=/usr \
	--libdir=/usr/lib/performance \
	--includedir=/usr/include/performance
endif

.PHONY: gasnet-bootstrap gasnet
.PHONY: gasnet-config-debug-force gasnet-config-debug gasnet-build-debug gasnet-debug
.PHONY: gasnet-config-perf-force gasnet-config-perf gasnet-build-perf gasnet-perf
.PHONY: gasnet-clean gasnet-distclean gasnet-mrproper

gasnet-bootstrap:
	@if [ ! -r $(GASNET_DIR)/configure ]; then \
	  cd $(GASNET_DIR); \
	  ./Bootstrap ;\
	fi

gasnet-config-debug-force: gasnet-bootstrap
	mkdir -p $(GASNET_BUILD_DIR_DEBUG)
	cd $(GASNET_BUILD_DIR_DEBUG); ../cross-configure-$(MODE)-axiom \
	  --with-axiom-sysroot=$(TARGET_DIR) \
	  $(GASNET_CONFIG_OTHER_DEBUG_ARGS) \
	  --enable-udp \
	  --enable-debug \
	  --enable-trace \
	  --enable-backtrace

gasnet-config-debug: libs
	@if [ ! -r "$(GASNET_BUILD_DIR_DEBUG)/config.status" ]; then \
	  $(MAKE) gasnet-config-debug-force ;\
	fi

gasnet-build-debug: gasnet-config-debug prepare-install
	@$(MAKE) -C $(GASNET_BUILD_DIR_DEBUG)

gasnet-debug: gasnet-build-debug
	$(SUDO) $(MAKE) -C $(GASNET_BUILD_DIR_DEBUG) DESTDIR=$(TARGET_DIR) install

gasnet-config-perf-force:
	mkdir -p $(GASNET_BUILD_DIR_PERF)
	cd $(GASNET_BUILD_DIR_PERF); ../cross-configure-$(MODE)-axiom \
	  --with-axiom-sysroot=$(TARGET_DIR) \
	  $(GASNET_CONFIG_OTHER_PERF_ARGS) \
	  --enable-udp \
	  --disable-debug

gasnet-config-perf: gasnet-bootstrap libs
	if [ ! -r "$(GASNET_BUILD_DIR_PERF)/config.status" ]; then \
	  $(MAKE) gasnet-config-perf-force ;\
	fi

gasnet-build-perf: gasnet-config-perf prepare-install
	@$(MAKE) -C $(GASNET_BUILD_DIR_PERF)

gasnet-perf: gasnet-build-perf
	$(SUDO) $(MAKE) -C $(GASNET_BUILD_DIR_PERF) DESTDIR=$(TARGET_DIR) install

gasnet: gasnet-debug gasnet-perf

gasnet-clean:
	@for DIR in $(GASNET_BUILD_DIR_DEBUG) $(GASNET_BUILD_DIR_PERF); do \
	  [ -d $${DIR} ] && \
	  $(MAKE) -C $${DIR} clean ;\
	done

gasnet-distclean:
	@for DIR in $(GASNET_BUILD_DIR_DEBUG) $(GASNET_BUILD_DIR_PERF); do \
	  [ -d $${DIR} ] && \
	  rm -r -f  $${DIR};\
	done

gasnet-mrproper: gasnet-distclean
	@if [ -r $(GASNET_DIR)/configure ]; then \
	  cd $(GASNET_DIR);\
	  ./unBootstrap;\
	fi

help::
	@echo "GASNET"
	@echo "  gasnet               Build all gasnet libraries"
	@echo "  gasnet-clean         Clean gasnet libraries"
	@echo "  gasnet-distclean     Clean gasnet libraries including configuration"
	@echo "  gasnet-mrproper      Clean gasnet libraries including configuration and autoconfig"
	@echo "  "
	@echo "  gasnet-boostrap           Gasnet configure bootstrap"
	@echo "  gasnet-config-debug-force Force configuration of debug libraries"
	@echo "  gasnet-config-debug       Configure debug libraries"
	@echo "  gasnet-build-debug        Compile debug libraries"
	@echo "  gasnet-debug              Install debug libraries"
	@echo "  gasnet-config-perf-force  Force configuration of performace libraries"
	@echo "  gasnet-config-perf        Configure performance libraries"
	@echo "  gasnet-build-perf         Compile performance libraries"
	@echo "  gasnet-perf               Install performance libraries"
	@echo " "

#
# NANOX
#

.PHONY: nanox-bootstrap
.PHONY: nanox-config-debug nanox-config-debug-force nanox-build-debug nanox-debug
.PHONY: nanox-config-perf nanox-config-perf-force nanox-build-perf nanox-perf
.PHONY: nanox-config-instr nanox-config-instr-force nanox-build-instr nanox-instr
.PHONY: nanox
.PHONY: nanox-instr-clean nanox-debug-clean nanox-perf-clean nanox-clean
.PHONY: nanox-instr-distclean nanox-debug-distclean nanox-perf-distclean nanox-distclean
.PHONY: nanox-mrproper

NANOX_PERF_VARS:=CXXFLAGS="-std=c++11"
NANOX_DEBUG_VARS:=$(NANOX_PERF_VARS)
ifeq ($(MODE),x86)
NANOX_INSTR_VARS:=$(NANOX_PERF_VARS)
else
NANOX_INSTR_VARS:=CXXFLAGS="-std=c++11 -Wno-error -I$(LINARO)/sysroot/usr/include -I$(ROOTFSTMP)/usr/include/aarch64-linux-gnu"
endif

nanox-bootstrap:
	@if [ ! -r $(NANOX_DIR)/configure ]; then \
	  cd $(NANOX_DIR);\
	  ./bootstrap;\
	fi

ifeq ($(DISABLE_INSTR),1)
nanox: nanox-debug nanox-perf
else
nanox: nanox-debug nanox-perf nanox-instr
endif

# nanox debug libraries

nanox-config-debug-force: nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_DEBUG)
	cd $(NANOX_BUILD_DIR_DEBUG); \
	  PATH=$(LINARO_PATH) \
	  LDFLAGS=-Wl,--build-id=uuid \
	  ROOTFS=$(TARGET_DIR) \
	  $(NANOX_DEBUG_VARS) \
	  ../configure \
            --prefix=$(DEFAULT_PREFIX) \
	    --build=$(SYS_COMP_ID) --host=$(SYS_RUN_ID) --target=$(SYS_RUN_ID) \
	    --with-gasnet \
	    --with-gasnet-include=$(TARGET_DIR)$(DEFAULT_PREFIX)/include/debug \
	    --with-gasnet-lib=$(TARGET_DIR)$(DEFAULT_PREFIX)/lib/debug \
	    --disable-performance \
	    --disable-instrumentation \
	    --disable-instrumentation-debug

nanox-config-debug:
	@if [ ! -r "$(NANOX_BUILD_DIR_DEBUG)/config.status" ]; then \
	  $(MAKE) nanox-config-debug-force; \
	fi

nanox-build-debug: nanox-config-debug prepare-install
	@PATH=$(LINARO_PATH) \
	    $(MAKE) $(NANOX_DEBUG_VARS) -C $(NANOX_BUILD_DIR_DEBUG) \
	    -j$(BUILDCPUS) 

nanox-debug: nanox-build-debug
	@PATH=$(LINARO_PATH) $(SUDO) \
	    $(MAKE) $(NANOX_DEBUG_VARS) -C $(NANOX_BUILD_DIR_DEBUG) \
	    DESTDIR=$(TARGET_DIR) install

# nanox performance libraries

nanox-config-perf-force: nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_PERF)
	cd $(NANOX_BUILD_DIR_PERF); \
	  PATH=$(LINARO_PATH) \
	  LDFLAGS=-Wl,--build-id=uuid \
	  ROOTFS=$(TARGET_DIR) \
	  $(NANOX_PERF_VARS) \
	  ../configure \
            --prefix=$(DEFAULT_PREFIX) \
	    --build=$(SYS_COMP_ID) --host=$(SYS_RUN_ID) --target=$(SYS_RUN_ID) \
	    --with-gasnet \
	    --with-gasnet-include=$(TARGET_DIR)$(DEFAULT_PREFIX)/include/performance \
	    --with-gasnet-lib=$(TARGET_DIR)$(DEFAULT_PREFIX)/lib/performance \
	    --disable-debug \
	    --disable-instrumentation \
	    --disable-instrumentation-debug

nanox-config-perf:
	@if [ ! -r "$(NANOX_BUILD_DIR_PERF)/config.status" ]; then \
	  $(MAKE) nanox-config-perf-force; \
	fi

nanox-build-perf: nanox-config-perf prepare-install
	@PATH=$(LINARO_PATH) \
	    $(MAKE) $(NANOX_PERF_VARS) -C $(NANOX_BUILD_DIR_PERF) \
	    -j$(BUILDCPUS) 

nanox-perf: nanox-build-perf
	@PATH=$(LINARO_PATH) $(SUDO) \
	    $(MAKE) $(NANOX_PERF_VARS) -C $(NANOX_BUILD_DIR_PERF) \
	    DESTDIR=$(TARGET_DIR) install

# nanox instrumentation libraries

nanox-config-instr-force: extrae nanox-bootstrap
	@mkdir -p $(NANOX_BUILD_DIR_INSTR)
	cd $(NANOX_BUILD_DIR_INSTR); \
	  PATH=$(LINARO_PATH) \
	  LDFLAGS=-Wl,--build-id=uuid \
	  ROOTFS=$(TARGET_DIR) \
	  $(NANOX_INSTR_VARS) \
	  ../configure \
            --prefix=$(DEFAULT_PREFIX) \
	    --build=$(SYS_COMP_ID) --host=$(SYS_RUN_ID) --target=$(SYS_RUN_ID) \
	    --with-gasnet \
	    --with-gasnet-include=$(TARGET_DIR)$(DEFAULT_PREFIX)/include/performance \
	    --with-gasnet-lib=$(TARGET_DIR)$(DEFAULT_PREFIX)/lib/performance \
	    --disable-debug \
	    --disable-performance \
	    --enable-instrumentation \
	    --with-extrae=${TARGET_DIR}/usr

nanox-config-instr:
	@if [ ! -r "$(NANOX_BUILD_DIR_INSTR)/config.status" ]; then \
	  $(MAKE) nanox-config-instr-force; \
	fi

nanox-build-instr: nanox-config-instr prepare-install
	@PATH=$(LINARO_PATH) \
	    $(MAKE) $(NANOX_INSTR_VARS) -C $(NANOX_BUILD_DIR_INSTR) \
	    -j$(BUILDCPUS)

nanox-instr: nanox-build-instr
	@PATH=$(LINARO_PATH) $(SUDO) \
	    $(MAKE) $(NANOX_INSTR_VARS) -C $(NANOX_BUILD_DIR_INSTR) \
	    DESTDIR=$(TARGET_DIR) install

# clenup

nanox-instr-clean:
	@[ -d $(NANOX_BUILD_DIR_INSTR) ] && make -C $(NANOX_BUILD_DIR_INSTR) clean

nanox-debug-clean:
	@[ -d $(NANOX_BUILD_DIR_DEBUG) ] && make -C $(NANOX_BUILD_DIR_DEBUG) clean

nanox-perf-clean:
	@[ -d $(NANOX_BUILD_DIR_PERF) ] && make -C $(NANOX_BUILD_DIR_PERF) clean

nanox-clean: nanox-perf-clean nanox-debug-clean nanox-instr-clean

nanox-instr-distclean:
	@[ -d $(NANOX_BUILD_DIR_INSTR) ] && rm -rf $(NANOX_BUILD_DIR_INSTR) || true

nanox-debug-distclean:
	@[ -d $(NANOX_BUILD_DIR_DEBUG) ] && rm -rf $(NANOX_BUILD_DIR_DEBUG) || true

nanox-perf-distclean:
	@[ -d $(NANOX_BUILD_DIR_PERF) ] && rm -rf $(NANOX_BUILD_DIR_PERF) || true

nanox-distclean: nanox-instr-distclean nanox-perf-distclean nanox-debug-distclean

nanox-mrproper: nanox-distclean
	@if [ -r $(NANOX_DIR)/configure ]; then \
	    rm $(NANOX_DIR)/configure;\
	fi

# help

help::
	@echo "NANOX (OMPSS)"
	@echo "  nanox               Build Nanos++ libraries"
	@echo "  nanox-clean         Clean Nanos++ libraries"
	@echo "  nanox-distclean     Clean Nanos++ libraries including configuration"
	@echo "  nanox-mrproper      Clean Nanos++ libraries including configuration and autoconfig"
	@echo " "
	@echo "  nanox-bootstrap          Configuration bootsrap"
	@echo "  nanox-config-debug-force Force configuration debug library"
	@echo "  nanox-config-perf-force  Force configuration performace library"
	@echo "  nanox-config-instr-force Force configuration performace library with Extrae"
	@echo "  nanox-config-debug       Configure debug library"
	@echo "  nanox-config-perf        Configure performace library"
	@echo "  nanox-config-instr       Configure performace library with Extrae"
	@echo "  nanox-build-debug        Build debug library"
	@echo "  nanox-build-perf         Build performance library"
	@echo "  nanox-build-instr        Build performance library with Extrae"
	@echo "  nanox-debug              Install debug library"
	@echo "  nanox-perf               Install performance library"
	@echo "  nanox-instr         	  Install performance library with Extrae"
	@echo " "

#
# MCXX
#

.PHONY: mcxx-boostrap
.PHONY: mcxx-config mcxx-config-force mcxx-build mcxx
.PHONY: mcxx-clean mcxx-distclean mcxx-mrproper

mcxx-bootstrap:
	@if [ ! -r $(MCXX_DIR)/configure ]; then \
	  cd $(MCXX_DIR);\
	  autoreconf -fiv;\
	fi

# mcxx host (i.e. run on cross build system)

MCXX_CONFIG_PARAMS:=\
	--with-nanox=$(TARGET_DIR)/usr \
	--enable-ompss \
	--disable-float128 \
	--disable-int128 \
	--disable-fortran-tests \
	--disable-vectorization \
	--disable-analysis

mcxx-config-force: mcxx-bootstrap
	mkdir -p $(MCXX_BUILD_DIR)
	cd $(MCXX_BUILD_DIR); \
	  PATH=$(LINARO_PATH) \
	  ../configure \
	    --prefix=$(LINARO_HOST_DIR)$(DEFAULT_PREFIX) \
	    --build=$(SYS_COMP_ID) --host=$(SYS_COMP_ID) --target=$(SYS_RUN_ID) \
	    $(MCXX_CONFIG_PARAMS)

mcxx-config:
	@if [ ! -r "$(MCXX_BUILD_DIR)/config.status" ]; then \
	  $(MAKE) mcxx-config-force; \
	fi

mcxx-build: mcxx-config
	PATH=$(LINARO_PATH) $(MAKE) -C $(MCXX_BUILD_DIR) -j$(BUILDCPUS)

mcxx: mcxx-build
	PATH=$(LINARO_PATH) $(MAKE) -C $(MCXX_BUILD_DIR) DESTDIR= install

# mcxx native (i.e. should run on axiom board)
# TO FIX! DOES NOT WORK!

define MCXX_CONFIG_NATIVE_VARS_OLD=
  CXXFLAGS="\
    -std=gnu++11 \
    -I$(TARGET_DIR)/usr/include \
    -I$(TARGET_DIR)/usr/include/aarch64-linux-gnu \
   " \
   CFLAGS="\
     -I$(TARGET_DIR)/usr/include \
     -I$(TARGET_DIR)/usr/include/aarch64-linux-gnu \
   " \
   LDFLAGS="\
     -L$(TARGET_DIR)/lib \
     -L$(TARGET_DIR)/lib/aarch64-linux-gnu \
     -L$(TARGET_DIR)/usr/lib \
     -L$(TARGET_DIR)/usr/lib/aarch64-linux-gnu \
   "
endef

define MCXX_CONFIG_NATIVE_VARS=
  CXXFLAGS="\
    -std=gnu++11 \
    --sysroot=$(TARGET_DIR) \
   " \
   CFLAGS="\
    --sysroot=$(TARGET_DIR) \
   " \
   LDFLAGS="\
    --sysroot=$(TARGET_DIR) \
   "
endef

mcxx-config-native-force: mcxx-bootstrap
	@mkdir -p $(MCXX_BUILD_NATIVE_DIR)
	cd $(MCXX_BUILD_NATIVE_DIR); \
	  PATH=$(LINARO_PATH) \
	  $(MCXX_CONFIG_NATIVE_VARS) \
	  ../configure \
	    --with-sysroot=$(TARGET_DIR) \
	    --prefix=$(DEFAULT_PREFIX) \
	    --build=$(SYS_COMP_ID) --host=$(SYS_RUN_ID) --target=$(SYS_RUN_ID) \
	    $(MCXX_CONFIG_PARAMS)

mcxx-config-native:
	@if [ ! -r "$(MCXX_BUILD_NATIVE_DIR)/config.status" ]; then \
	  $(MAKE) mcxx-config-native-force; \
	fi

mcxx-build-native: mcxx-config-native
	PATH=$(LINARO_PATH) $(MAKE) $(MCXX_CONFIG_NATIVE_VARS) -C $(MCXX_BUILD_NATIVE_DIR) -j$(BUILDCPUS)

mcxx-native:
	PATH=$(LINARO_PATH) \
	  $(SUDO) $(MAKE) -C $(MCXX_BUILD_NATIVE_DIR) \
	  DESTDIR=$(TARGET_DIR) install

# cleanup

.PHONY: mcxx-clean mcxx-distclean mcxx-mrproper

mcxx-clean:
	@[ -d $(MCXX_BUILD_DIR) ] && make -C $(MCXX_BUILD_DIR) clean || true
	@[ -d $(MCXX_BUILD_NATIVE_DIR) ] && make -C $(MCXX_BUILD_NATIVE_DIR) clean || true

mcxx-distclean:
	@[ -d $(MCXX_BUILD_DIR) ] && rm -rf $(MCXX_BUILD_DIR) || true
	@[ -d $(MCXX_BUILD_NATIVE_DIR) ] && rm -rf $(MCXX_BUILD_NATIVE_DIR) || true

mcxx-mrproper: mcxx-distclean
	@if [ -r $(MCXX_DIR)/configure ]; then \
	    rm $(MCXX_DIR)/configure;\
	fi

# help

help::
	@echo "MCXX (OMPSS)"
	@echo "  mcxx              Build and install Mercurium source cross compiler (run on HOST)"
	@echo "  clean-mcxx        Clean Mercurium"
	@echo "  distclean-mcxx    Clean Mercurium includind configuration"
	@echo ""
	@echo "  mcxx-bootstrap           Configuration bootstrap"
	@echo "  mcxx-config-force        Force the configuration of Mercurium"
	@echo "  mcxx-config              Mercurium configuration"
	@echo "  mcxx-build               Compile Mercurium"
	@echo " "

#
# OMPSS
#

.PHONY: ompss ompss-clean ompss-distclean ompss-mrproper

ompss: nanox mcxx

ompss-clean: nanox-clean mcxx-clean

ompss-distclean: nanox-distclean mcxx-distclean

ompss-mrproper: nanox-mrproper mcxx-mrproper

help::
	@echo "OMPSS"
	@echo "  ompss             Build OmpSs (Nanos++ and Mercurium)"
	@echo "  ompss-clean       Clean OmpSs"
	@echo "  ompss-distclean   Clean OmpSS including configuration"
	@echo "  ompss-mrproper    Clean OmpSS including configuration and autoconfig"
	@echo " "

#
# TEST
#

.PHONY: tests tests-axiom tests-gasnet tests-ompss
.PHONY: tests-clean tests-axiom-clean tests-gasnet-clean tests-ompss-clean

tests-axiom: prepare-install
	@$(MAKE) -C $(AXIOM_TESTS_DIR) build install

tests-axiom-clean:
	@$(MAKE) -C $(AXIOM_TESTS_DIR) clean

tests-ompss: prepare-install
	@$(MAKE) -C $(OMPSS_TESTS_DIR) build install

tests-ompss-clean:
	@$(MAKE) -C $(OMPSS_TESTS_DIR) clean

tests-gasnet: gasnet prepare-install
	@$(MAKE) -C $(GASNET_TESTS_DIR) build install

tests-gasnet-clean:
	@$(MAKE) -C $(GASNET_TESTS_DIR) clean

tests: tests-axiom tests-gasnet tests-ompss

tests-clean: tests-axiom-clean tests-gasnet-clean tests-ompss-clean

help::
	@echo "TEST"
	@echo "  tests-axiom        Build (and install) AXIOM API tests"
	@echo "  tests-gasnet       Build (and install) GASNet tests"
	@echo "  tests-ompss        Build (and install) OmpSS tests"
	@echo "  tests              Build all tests"
	@echo "  tests-gasnet-clean Clean gasnet tests"
	@echo "  tests-ompss-clean  Clean OMPSS tests"
	@echo "  tests-axiom-clean  Clean AXIOM tests"
	@echo "  test-clean         Clean all tests"
	@echo " "

#
# DEB packages
#

.PHONY: deb-prepare deb-unprepare deb-install deb-move deb deb-tests clean-deb distclean-deb

ifeq ($(MODE),x86)

# x86 deb-prepare deb-unprepare makefile targets

deb-prepare:
	mkdir -p $(AXIOM_DIR)/debs
	rm -f $(AXIOM_DIR)/*.deb
deb-prepare-force: deb-prepare
deb-unprepare:
	rm -f $(AXIOM_DIR)/*.deb

else

# aarch64 deb-prepare deb-unprepare makefile targets

ROOTFSDEBTMP:=$(AXIOM_DIR)/rootfs-deb-tmp
ROOTFSDEBWRK:=$(AXIOM_DIR)/rootfs-deb-wrk

deb-prepare: prepare-install
	@if ! findmnt -n $(ROOTFSDEBTMP) >/dev/null; then \
	  mkdir -p $(ROOTFSDEBTMP) ;\
	  mkdir -p $(ROOTFSDEBWRK) ;\
	  sudo mount -t overlay overlay \
	    -olowerdir=$(ROOTFS),upperdir=$(ROOTFSDEBTMP),workdir=$(ROOTFSDEBWRK) \
	    $(ROOTFSDEBTMP) ;\
	  cp $(shell which qemu-aarch64) $(ROOTFSDEBTMP)/usr/bin ;\
	  if [ ! -x $(ROOTFSDEBTMP)/usr/bin/qemu-aarch64 ]; then \
	    echo "$(ccred)WARNING: qemu-aarch64 NOT FOUND on system path$(ccend)" ;\
	  fi \
	fi
	rm -f $(AXIOM_DIR)/*.deb
	rm -f $(ROOTFSDEBTMP)/*.deb

deb-prepare-force:
	-sudo umount $(ROOTFSDEBTMP)
	$(MAKE) deb-prepare

deb-unprepare:
	rm -f $(AXIOM_DIR)/*.deb
	rm -f $(ROOTFSDEBTMP)/*.deb
	-sudo umount $(ROOTFSDEBTMP)

deb-remove-tempdir: deb-unprepare
	sudo rm -r -f $(ROOTFSDEBTMP) $(ROOTFSDEBWRK)

endif

# used to make the debianization
ifeq ($(MODE),x86)
DEBENV := PATH=$(SCRIPTS_DIR)/bin:$(PATH) \
	fakeroot
else
DEBENV := PATH=$(SCRIPTS_DIR)/bin:$(LINARO_PATH) \
	ROOTFS=$(ROOTFSDEBTMP) \
	GCC_TARGET=arm64 \
	CC=$(LINARO_CC) \
	fakeroot
endif

# chroot to ROOTFS as user
# PLEASE DO NOT DELETE!
#rootfs-shell:
#	@FAIL=0 ;\
#	  [ -x /usr/bin/qemu-aarch64 ] || \
#	    { echo "$(ccred)/usr/bin/qemu-aarch64 not found!$(ccend)";\
#	    FAIL=1; } ;\
#	  [ -x $(ROOTFSDEBTMP)/usr/bin/qemu-aarch64 ] || \
#	    { echo "$(ccred)$(ROOTFSDEBTMP)/usr/bin/qemu-aarch64 not found!$(ccend)";\
#	    FAIL=1; } ;\
#	  [ -f /proc/sys/fs/binfmt_misc/qemu-aarch64 ] || \
#	    { echo "$(ccred)/proc/sys/fs/binfmt_misc/qemu-aarch64 not found!$(ccend)";\
#	    FAIL=1; } ;\
#	  [ -e /lib/ld-linux-aarch64.so.1 ] || \
#	    { echo "$(ccred)/lib/ld-linux-aarch64.so.1 not found!$(ccend)";\
#	    FAIL=1; } ;\
#	  [ -e $(ROOTFSDEBTMP)/lib/ld-linux-aarch64.so.1 ] || \
#	    { echo "$(ccred)$(ROOTFSDEBTMP)/lib/ld-linux-aarch64.so.1 not found!$(ccend)";\
#	    FAIL=1; } ;\
#	  [ $$FAIL -eq 0 ] && $(FAKEROOT) fakechroot chroot $(ROOTFSDEBTMP) /bin/bash

# install the generated DEB packages into the debian temp rootfs
# and save them into debs/ directory
deb-install:
	mkdir -p $(AXIOM_DIR)/debs
	for D in $(AXIOM_DIR)/*deb; do \
		DEB=`basename $$D`; \
		ST=`echo $$DEB | sed 's/_.*/_/'`; \
		rm -f $(AXIOM_DIR)/debs/$$ST*;\
	done
	cp $(AXIOM_DIR)/*deb $(AXIOM_DIR)/debs
ifeq ($(MODE),x86)
#
# X86
#
	sudo dpkg --install $(AXIOM_DIR)/*.deb
	rm -f $(AXIOM_DIR)/*.deb
else
#
# other
#
	cp $(AXIOM_DIR)/*deb $(ROOTFSDEBTMP)
	sudo chroot $(ROOTFSDEBTMP) /bin/bash -c "/usr/bin/dpkg --install /*.deb"
	rm -f $(ROOTFSDEBTMP)/*.deb
	rm -f $(AXIOM_DIR)/*.deb
endif

# save the generated DEB packages into the debs/ directory
deb-move:
	mkdir -p $(AXIOM_DIR)/debs
	for D in $(TESTS_DIR)/*deb; do \
		DEB=`basename $$D`; \
		ST=`echo $$DEB | sed 's/_.*/_/'`; \
		rm -f $(AXIOM_DIR)/debs/$$ST*;\
	done
	mv $(TESTS_DIR)/*deb $(AXIOM_DIR)/debs

.PHONY: _deb-kernel _deb-extrae _deb_allocator-drv _deb-allocato-lib
.PHONY: _deb_init-lib _deb-run-lib _deb_allocator _deb-apps _deb-gasnet _deb-nanox
.PHONY: _deb_meta

_deb-kernel:
# kernel modules
ifneq ($(MODE),x86)
	@echo "$(ccgreen)===DEBs for kernel modules [deb-kernel]$(ccend)"
	$(DEBENV) $(MAKE) -C $(SCRIPTS_DIR)/debian_kernel -f debian/rules binary
	mv -f $(SCRIPTS_DIR)/*.deb $(AXIOM_DIR)/
	$(MAKE) deb-install
endif
deb-kernel: deb-prepare _deb-kernel
_deb-meta:
# metapackages
	@echo "$(ccgreen)===DEBs metapackages [deb-meta]$(ccend)"
	$(DEBENV) $(MAKE) -C $(SCRIPTS_DIR)/debian_meta -f debian/rules binary
	mv -f $(SCRIPTS_DIR)/*deb $(AXIOM_DIR)/debs
deb-meta: deb-prepare _deb-meta
_deb-extrae:
# extrae
	@echo "$(ccgreen)===DEBs for extrae [deb-extrae]$(ccend)"
ifeq ($(DISABLE_INSTR),0)
	$(DEBENV) $(MAKE) -C $(EXTRAE_DIR) -f debian/rules binary
	$(MAKE) deb-install
else
	@echo "Instrumentation disabled"
endif
deb-extrae: deb-prepare _deb-extrae
_deb-allocator-drv:
# axiom allocator kernel module
	@echo "$(ccgreen)===DEBs for allocator kernel module [deb-allocator-drv]$(ccend)"
	$(DEBENV) $(MAKE) -C $(ALLOCATOR_DRV_DIR) -f debian/rules binary
	$(MAKE) deb-install
deb-allocator-drv: deb-prepare _deb-allocator-drv
_deb-nic:
# axiom user library & kernel module
	@echo "$(ccgreen)===DEBs for axiom user library (and kernel module) [deb-nic]$(ccend)"
	$(DEBENV) $(MAKE) -C $(NIC_DIR) -f debian/rules binary
	$(MAKE) deb-install
deb-nic: deb-prepare _deb-nic
_deb-allocator-lib:
# axiom allocator library
	@echo "$(ccgreen)===DEBs for allocator library [deb-allocator-lib]$(ccend)"
	$(DEBENV) $(MAKE) -C $(ALLOCATOR_LIB_DIR) -f debian/rules binary
	$(MAKE) deb-install
deb-allocator-lib: deb-prepare _deb-allocator-lib
_deb-init-lib:
# axiom init library
	@echo "$(ccgreen)===DEBs for axiom init library [deb-init-lib]$(ccend)"
	$(DEBENV) $(MAKE) -C $(APPS_DIR)/axiom-init -f debian/rules binary
	mv $(APPS_DIR)/*.deb $(AXIOM_DIR)
	$(MAKE) deb-install
deb-init-lib: deb-prepare _deb-init-lib
_deb-run-lib:
# axiom run library
	@echo "$(ccgreen)===DEBs for axiom run library [deb-run-lib]$(ccend)"
	$(DEBENV) $(MAKE) -C $(APPS_DIR)/axiom-run -f debian/rules binary
	mv $(APPS_DIR)/*.deb $(AXIOM_DIR)
	$(MAKE) deb-install
deb-run-lib: deb-prepare _deb-run-lib
_deb-allocator:
# memory allocator
	@echo "$(ccgreen)===DEBs for axiom allocator [deb-allocator]$(ccend)"
	$(DEBENV) $(MAKE) -C $(ALLOCATOR_DIR) -f debian/rules binary
	$(MAKE) NCONF=1 deb-install
deb-allocator: deb-prepare _deb-allocator
_deb-apps:
# axiom apps
	@echo "$(ccgreen)===DEBs for axiom system applications [deb-apps]$(ccend) "
	$(DEBENV) $(MAKE) -C $(APPS_DIR) -f debian/rules binary
	$(MAKE) deb-install
deb-apps: deb-prepare _deb-apps
_deb-gasnet:
# gasnet
	@echo "$(ccgreen)===DEBs for gasnet [deb-gasnet]$(ccend)"
	$(DEBENV) $(MAKE) -C $(GASNET_DIR) -f debian/rules binary
	$(MAKE) deb-install
deb-gasnet: deb-prepare _deb-gasnet
_deb-nanox:
# nanox
	@echo "$(ccgreen)===DEBs for nanox [deb-nanox]$(ccend)"
	$(DEBENV) $(MAKE) -C $(NANOX_DIR) -f debian/rules binary
	$(MAKE) deb-install
deb-nanox: deb-prepare _deb-nanox
_deb-mcxx:
# mcxx
	@echo "$(ccgreen)===DEBs for mcxx [deb-mcxx]$(ccend)"
ifeq ($(MODE),x86)
	$(DEBENV) fakeroot $(MAKE) -C $(MCXX_DIR) -f debian/rules binary
	$(MAKE) deb-install
endif
deb-mcxx: deb-prepare _deb-mcxx

.PHONY: deb-kernel deb-extrae deb_allocator-drv deb-allocato-lib
.PHONY: deb_init-lib deb-run-lib deb_allocator deb-gasnet deb-nanox

deb-warn:
	@echo "$(ccred)Remeber that to make deb packages all must be already build$(ccend)"

deb-warn2:
	@echo "$(ccgreen)Packages for test not done.  Use 'make deb-tests' to DEBIANize them$(ccend)"

deb: deb-warn              \
	deb-prepare        \
	_deb-kernel        \
	_deb-meta          \
	_deb-extrae        \
	_deb-allocator-drv \
	_deb-nic           \
	_deb-allocator-lib \
	_deb-init-lib      \
	_deb-run-lib       \
	_deb-allocator     \
	_deb-apps          \
	_deb-gasnet        \
	_deb-nanox         \
	_deb-mcxx          \
	deb-warn2

.PHONY: _deb-tests-ompss _deb-tests-gasnet _deb-tests-axiom

_deb-tests-ompss:
# ompss axiom tests
	@echo "$(ccgreen)===DEBs for ompss test [deb-tests-ompss]$(ccend)"
	$(DEBENV) $(MAKE) -C $(OMPSS_TESTS_DIR) -f debian/rules binary
	$(MAKE) deb-move
deb-tests-ompss: deb-prepare _deb-tests-ompss

_deb-tests-gasnet:
# gasnet axiom tests
ifneq ($(MODE),x86)
	@echo "$(ccgreen)===DEBs for gasnet test [deb-tests-gasnet]$(ccend)"
	$(DEBENV) $(MAKE) -C $(GASNET_TESTS_DIR) -f debian/rules binary
	$(MAKE) deb-move
endif
deb-tests-gasnet: deb-prepare _deb-tests-gasnet

_deb-tests-axiom:
# axiom tests
	@echo "$(ccgreen)===DEBs for axiom test [deb-tests-axiom]$(ccend)"
	$(DEBENV) $(MAKE) -C $(AXIOM_TESTS_DIR) -f debian/rules binary
	$(MAKE) deb-move
deb-tests-axiom: deb-prepare _deb-tests-axiom

.PHONY: deb-tests-ompss deb-tests-gasnet deb-tests-axiom

deb-tests: deb-prepare \
	deb-tests-ompss \
	deb-tests-gasnet \
	deb-tests-axiom

deb-clean: deb-unprepare
	$(DEBENV) $(MAKE) -C $(SCRIPTS_DIR)/debian_kernel -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(SCRIPTS_DIR)/debian_meta -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(EXTRAE_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(NIC_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(ALLOCATOR_DRV_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(ALLOCATOR_LIB_DIR) -f debian/rules  clean
	$(DEBENV) $(MAKE) -C $(APPS_DIR)/axiom-init -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(APPS_DIR)/axiom-run -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(ALLOCATOR_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(APPS_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(GASNET_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(NANOX_DIR) -f debian/rules clean
ifeq ($(MODE),x86)
	$(DEBENV) $(MAKE) -C $(MCXX_DIR) -f debian/rules clean
endif
	$(DEBENV) $(MAKE) -C $(AXIOM_TESTS_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(GASNET_TESTS_DIR) -f debian/rules clean
	$(DEBENV) $(MAKE) -C $(OMPSS_TESTS_DIR) -f debian/rules clean
	-rm -f $(AXIOM_DIR)/*.deb

deb-distclean: deb-clean extrae-distclean gasnet-distclean nanox-distclean
	test -e $(NIC_DIR)/Makefile && $(MAKE) -C $(NIC_DIR) distclean || true
	test -e $(ALLOC_DRV_DIR)/Makefile && $(MAKE) -C $(ALLOCATOR_DRV_DIR) distclean || true
	test -e $(ALLOC_LIB_DIR)/Makefile && $(MAKE) -C $(ALLOCATOR_LIB_DIR) distclean || true
	test -e $(ALLOCATOR_DIR)/Makefile && $(MAKE) -C $(ALLOCATOR_DIR) distclean || true
	test -e $(APPS_DIR)/Makefile && $(MAKE) -C $(APPS_DIR) distclean || true

help::
	@echo "DEBIANization"
	@echo "  deb            Build debian packages"
	@echo "  deb-tests      Build test debian packages"
	@echo " "
	@echo "  deb-kernel        Build package for kernel"
	@echo "  deb-meta          Build package for metapackages"
	@echo "  deb-extrae        Build package for extrae"
	@echo "  deb-nic           Build package for Axiom nic driver"
	@echo "  deb-apps          Build package for Axiom applications"
	@echo "  deb-allocator-drv Build package for memory driver"
	@echo "  deb-allocator-lib Build package for memory library"
	@echo "  deb-init-lib      Build package for init library"
	@echo "  deb-run-lib       Build package for run library"
	@echo "  deb-allocator     Build package for memory allocator"
	@echo "  deb-gasnet        Build package for Axiom Gasnet conduit"
	@echo "  deb-nanox         Build package for Nanos++"
	@echo "  deb-mcxx          Build package for Mercurium (DISABLED)"
	@echo "  PS: packages build require a prior complete build"
	@echo " "
	@echo "  deb-tests-axiom   Build package for Axiom tests"
	@echo "  deb-tests-gasnet  Build package for Gasnet tests"
	@echo "  deb-tests-ompss   Build package for OMPSS@Cluster tests"
	@echo " "
	@echo "  deb-clean         Clean all deb build (including tests packages)"
	@echo "  deb-ditsclean     Clean all deb build and compilation"
	@echo " "

#
# general help
#

help::
	@echo "GENERAL"
	@echo "  build         Build libraries and axiom applications"
	@echo "  all           As 'build' including gasnet, ompss and tests (DEFAULT target)"
	@echo "  gasnet        Build gasnet (libraries must already be built)"
	@echo "  nanox         Build nanox (libraries and nanox must already be built)"
	@echo "  clean         Clean projects (but no gasnet, ompss and test)"
	@echo "  clean-all     Clean projects including subproject ganset, ompss and tests"
	@echo "  mount-temp    Mount needed overlayfs"
	@echo "  umount-temp   Umount used overlayfs"
	@echo "  help          This help"
	@echo " "
	@echo "  distclean     Clean projects including configuration"
	@echo "  mrproper      Clean projects including configuration and autoconfig"
	@echo "  distclean-all Clean projects including configuration and subproject gasnet,ompss and tests"
	@echo "  mrproper-all  Clean projects including configuration and autoconfig and subproject gasnet,ompss and tests"
	@echo " "
