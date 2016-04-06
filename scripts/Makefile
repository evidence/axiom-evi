DATE := $(shell date +%Y%m%d_%H%M%S)-
GIT_DESC := $(shell git describe --tags)
OUTPUT_TGZ := $(DATE)axiom-nic-evisim-$(GIT_DESC)

VMS := 2

IMAGES := ../axiom-evi-buildroot/output/images
QEMU := ../axiom-evi-qemu/aarch64-softmmu/qemu-system-aarch64
SWITCH := ../axiom-evi-nic/axiom_switch/axiom_switch
SWITCH_PARAM := -t 1 -n $(VMS) -v
OUTPUT_DIR := output
LOG_FILE := make.log

ifdef SWITCH
SWITCH_RUN = $(SWITCH) $(SWITCH_PARAM) >> $(LOG_FILE) 2>&1 &
else
SWITCH_RUN=
SWITCH_PARAM=
endif

IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))



.PHONY: $(RUNS) run switch stop archive clean clean-log

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

archive: clean
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

log:
	@tail -f -n +1 $(LOG_FILE)

clean-log:
	-@rm $(LOG_FILE)

clean: clean-log
	rm -rf $(OUTPUT_DIR) $(OUTPUT_TGZ).tgz
