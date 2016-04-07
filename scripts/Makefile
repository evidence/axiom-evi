DATE := $(shell date +%Y%m%d_%H%M%S)-

# number of Virtual Machines to run
VMS := 2

GIT_DESC := $(shell git describe --tags)
OUTPUT_TGZ := $(DATE)axiom-nic-evisim-$(GIT_DESC)

QEMU_DIR := ../axiom-evi-qemu
BUILDROOT_DIR := ../axiom-evi-buildroot
EVI_NIC_DIR := ../axiom-evi-nic
EVI_APPS_DIR := ../axiom-evi-apps
DFLAGS :=
#DFLAGS := -g -PDEBUG

IMAGES := $(BUILDROOT_DIR)/output/images
QEMU := $(QEMU_DIR)/aarch64-softmmu/qemu-system-aarch64
SWITCH :=$(EVI_NIC_DIR)/axiom_switch/axiom_switch
SWITCH_PARAM := -t 1 -n $(VMS) -v
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


.PHONY: $(RUNS) run switch stop archive clean clean-log clean-evi install $(QEMU_DIR) $(BUILDROOT_DIR) $(EVI_NIC_DIR) $(EVI_APPS_DIR)


all: $(EVI_NIC_DIR) $(EVI_APPS_DIR) $(BUILDROOT_DIR) $(QEMU_DIR)

configure:
	cd $(BUILDROOT_DIR) && make evi_zc706_defconfig && make uboot-patch
	cd $(QEMU_DIR) && ./axiom-configure.sh

$(QEMU_DIR)*:
	cd $(QEMU_DIR) && make -j5

$(BUILDROOT_DIR):
	cd $(BUILDROOT_DIR) && make

$(EVI_NIC_DIR)*:
	cd $(EVI_NIC_DIR) && make DFLAGS="$(DFLAGS)" install

$(EVI_APPS_DIR)*:
	cd $(EVI_APPS_DIR) && make DFLAGS="$(DFLAGS)" install


archive: clean-output
	mkdir -p $(OUTPUT_DIR)/images
	cp $(QEMU) $(OUTPUT_DIR)/
	cp $(SWITCH) $(OUTPUT_DIR)/
	cp $(IMAGES)/*dtb $(OUTPUT_DIR)/images/
	cp $(IMAGES)/*img $(OUTPUT_DIR)/images/
	cp $(IMAGES)/uImage $(OUTPUT_DIR)/images/
	cp $(IMAGES)/*uboot $(OUTPUT_DIR)/images/
	cp README.archive $(OUTPUT_DIR)/README
	cp Makefile.archive $(OUTPUT_DIR)/Makefile
	cp start_qemu.sh.archive $(OUTPUT_DIR)/start_qemu.sh
	tar -zcvf $(OUTPUT_TGZ).tgz $(OUTPUT_DIR) --transform s/$(OUTPUT_DIR)/$(OUTPUT_TGZ)/
	mv $(OUTPUT_TGZ).tgz $(ARCHIVE_DIR)


run: clean-log stop switch $(RUNS)

$(RUNS): run%:
	@echo "starting qemu[$*]: $(QEMU)" | tee $(LOG_FILE)
	@./start_qemu.sh --id $* --qemu ${QEMU} >> $(LOG_FILE) 2>&1 &

stop:
	-@killall $(QEMU) 2> /dev/null
	-@killall $(SWITCH) 2> /dev/null

switch:
	@echo "starting switch: $(SWITCH) $(SWITCH_PARAM)" | tee $(LOG_FILE)
	@$(SWITCH_RUN)

log:
	@tail -f -n +1 $(LOG_FILE)


clean-log:
	-@rm $(LOG_FILE)

clean-evi:
	cd $(EVI_NIC_DIR) && make clean
	cd $(EVI_APPS_DIR) && make clean

clean-output:
	rm -rf $(OUTPUT_DIR)


clean: clean-log clean-evi clean-output
