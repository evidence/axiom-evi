DATE := $(shell date +%Y%m%d_%H%M%S)-
MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))


# number of Virtual Machines to run
VMS := 2

GIT_DESC := $(shell git describe --tags | sed s/axiom-//g)
OUTPUT_TGZ := $(DATE)axiom-nic-evisim-$(GIT_DESC)
release: OUTPUT_TGZ := axiom-nic-evisim-$(GIT_DESC)

QEMU_DIR := $(MKFILE_DIR)/../axiom-evi-qemu
BUILDROOT_DIR := $(MKFILE_DIR)/../axiom-evi-buildroot
ARM_FIRMWARE_DIR := $(MKFILE_DIR)/../arm-trusted-firmware
QEMU_DTS_DIR := $(MKFILE_DIR)/./buildroot/qemu-dts
EVI_NIC_DIR := $(MKFILE_DIR)/../axiom-evi-nic
EVI_APPS_DIR := $(MKFILE_DIR)/../axiom-evi-apps
GASNET_DIR := $(MKFILE_DIR)/../axiom-evi-gasnet
GASNET_BUILD_DIR := $(MKFILE_DIR)/../axiom-evi-gasnet/build
EVI_ALLOC_DRV := $(MKFILE_DIR)/../axiom-evi-allocator-drv
EVI_ALLOC_LIB := $(MKFILE_DIR)/../axiom-evi-allocator-lib
ALLOCATOR := $(MKFILE_DIR)/../axiom-allocator
OVERLAY := $(MKFILE_DIR)/buildroot/overlay

AXIOM_DTB := axiom-board.dtb
DFLAGS := -g
debug: DFLAGS := -g -DPDEBUG

#CCARCH := arm
CCARCH := aarch64
IMAGES := $(BUILDROOT_DIR)/output/images
QEMU := $(QEMU_DIR)/aarch64-softmmu/qemu-system-aarch64
CCPREFIX := ${BUILDROOT_DIR}/output/host/usr/bin/$(CCARCH)-linux-
SWITCH :=$(EVI_NIC_DIR)/axiom_switch/axiom_switch
SWITCH_PARAM := -r -n $(VMS)
OUTPUT_DIR := output
ARCHIVE_DIR := archive
LOG_FILE := make.log

ifdef SWITCH
SWITCH_RUN = $(SWITCH) $(SWITCH_PARAM) >> $(LOG_FILE) 2>&1 &
else
SWITCH_RUN=
SWITCH_PARAM=
endif

IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))


.PHONY: $(RUNS) run help switch stop archive release clean clean-log clean-evi install netup netdown setupnet debug libs allocator gasnet $(QEMU_DIR) $(BUILDROOT_DIR) $(EVI_NIC_DIR) $(EVI_APPS_DIR) $(ARM_FIRMWARE_DIR) $(QEMU_DTS_DIR)

export PATH := $(BUILDROOT_DIR)/output/host/usr/bin/:$(PATH)

all: stop libs allocator $(EVI_NIC_DIR) $(EVI_APPS_DIR) $(BUILDROOT_DIR) $(ARM_FIRMWARE_DIR) $(QEMU_DTS_DIR) $(QEMU_DIR)

debug: all

configure: config-alloc
	cd $(BUILDROOT_DIR) && make evi_ultrascale_defconfig && make
	cd $(QEMU_DIR) && ./axiom_configure.sh

config-alloc:
	cd $(EVI_ALLOC_LIB) && autoreconf -fi && ./configure --host=aarch64-buildroot-linux-gnu --prefix=$(OVERLAY)/usr --disable-debug --enable-extld
	cd $(ALLOCATOR) && autoreconf -fi && ./configure --host=aarch64-buildroot-linux-gnu --prefix=$(OVERLAY)/usr --with-evi-allocator=$(OVERLAY)/usr --with-axiom-mem-dev=$(EVI_ALLOC_DRV)


$(QEMU_DIR)*:
	cd $(QEMU_DIR) && make -j5

$(BUILDROOT_DIR):
	cd $(BUILDROOT_DIR) && make

$(ARM_FIRMWARE_DIR):
	cd $(ARM_FIRMWARE_DIR) && make CROSS_COMPILE=$(CCPREFIX) RESET_TO_BL31=1 PLAT=zynqmp all fip

$(QEMU_DTS_DIR):
	cd $(QEMU_DTS_DIR) && make DTC=$(BUILDROOT_DIR)/output/host/usr/bin/linux-dtc

$(EVI_NIC_DIR)*:
	cd $(EVI_NIC_DIR) && make DFLAGS="$(DFLAGS)" install

$(EVI_APPS_DIR)*:
	cd $(EVI_APPS_DIR) && make DFLAGS="$(DFLAGS)" install

gasnet:
	@if [ ! -r $(GASNET_DIR)/configure ]; then \
		cd $(GASNET_DIR);\
		./Bootstrap;\
	fi
	mkdir -p $(GASNET_BUILD_DIR)
	@cd $(GASNET_BUILD_DIR); ../cross-configure-axiom --disable-debug
	@make -C $(GASNET_BUILD_DIR)
	@make -C $(GASNET_BUILD_DIR)/axiom-conduit tests-par

libs:
	cd $(EVI_NIC_DIR) && make DFLAGS="$(DFLAGS)" libs
	cd $(EVI_APPS_DIR) && make DFLAGS="$(DFLAGS)" libs

allocator: libs
	cd $(EVI_ALLOC_DRV) && make CCARCH="$(CCARCH)" install
	cd $(EVI_ALLOC_LIB) && make install
	cd $(ALLOCATOR) && make install

archive: clean-output
	mkdir -p $(OUTPUT_DIR)/images
	cp $(QEMU) $(OUTPUT_DIR)/
	cp $(SWITCH) $(OUTPUT_DIR)/
	cp $(IMAGES)/*dtb $(OUTPUT_DIR)/images/
	cp $(IMAGES)/*elf $(OUTPUT_DIR)/images/
	cp $(IMAGES)/*Image $(OUTPUT_DIR)/images/
	cp $(QEMU_DTS_DIR)/$(AXIOM_DTB) $(OUTPUT_DIR)/images/
	cp $(ARM_FIRMWARE_DIR)/build/zynqmp/release/bl31/bl31.elf $(OUTPUT_DIR)/images/
	cp README.archive $(OUTPUT_DIR)/README
	cp ../CHANGELOG $(OUTPUT_DIR)/CHANGELOG
	cp Makefile.archive $(OUTPUT_DIR)/Makefile
	cp start_qemu.sh.archive $(OUTPUT_DIR)/start_qemu.sh
	tar -zcvf $(OUTPUT_TGZ).tgz $(OUTPUT_DIR) --transform s/$(OUTPUT_DIR)/$(OUTPUT_TGZ)/
	mv $(OUTPUT_TGZ).tgz $(ARCHIVE_DIR)

release: clean all archive

help:
	@echo "Makefile targets:"
	@echo "run\t\t Start nodes and axiom switch"
	@echo "stop\t\t Stop nodes and axio switch"
	@echo "netup\t\t Setup network on host"
	@echo "netdown\t\t Clean network on host"
	@echo "NETWORK=1 run\t Start nodes with network enabled"
	@echo "setupnet\t Setup networks and axiom-init on nodes (nodes must be running)"
	@echo "gasnet\t\t Configure and build GASNet library (including tests)"

run: clean-log stop switch $(RUNS)

netup:
	@./setup_network_on_host.sh up

netdown:
	@./setup_network_on_host.sh down

setupnet:
	@./setup_network_on_nodes.sh

$(RUNS): run%:
	@echo "starting qemu[$*]: $(QEMU)" | tee -a $(LOG_FILE)
	@./start_qemu.sh --id $* --qemu ${QEMU} >> $(LOG_FILE) 2>&1 &

stop:
	@echo "stopping qemu and axiom_switch" | tee -a $(LOG_FILE)
	-@killall $(QEMU) 2> /dev/null
	-@killall $(SWITCH) 2> /dev/null

switch:
	@echo "starting switch: $(SWITCH) $(SWITCH_PARAM)" | tee -a $(LOG_FILE)
	@$(SWITCH_RUN)

log:
	@tail -f -n +1 $(LOG_FILE)


clean-log:
	-@rm $(LOG_FILE)

clean-evi: stop
	cd $(EVI_NIC_DIR) && make clean
	cd $(EVI_APPS_DIR) && make clean
	cd $(QEMU_DTS_DIR) && make clean
	cd $(EVI_ALLOC_DRV) && make CCARCH=$(CCARCH) clean
	cd $(EVI_ALLOC_LIB) && make clean
	cd $(ALLOCATOR) && make clean

clean-output:
	rm -rf $(OUTPUT_DIR)


clean: clean-log clean-evi clean-output
