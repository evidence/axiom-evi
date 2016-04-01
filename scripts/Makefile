VMS := 1

PREFIX := $(HOME)/usr/axiom
IMAGES := ../axiom-buildroot/output/images
QEMU := ../axiom-qemu/aarch64-softmmu/qemu-system-aarch64
SWITCH := ../axiom-nic/axiom_switch/axiom_switch
PWD := $(shell pwd)
OUTPUT_DIR := output
OUTPUT_TGZ := axiom-nic-evisim

IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))

.PHONY: $(RUNS) run switch stop archive clean

run: stop switch $(RUNS)


$(RUNS): run%:
	./start_qemu.sh --id $* &

stop:
	-killall $(QEMU)
	-killall $(SWITCH)

switch:
	$(SWITCH) $(VMS) &

archive: clean
	mkdir -p $(OUTPUT_DIR)/images
	cp $(QEMU) $(OUTPUT_DIR)/
	cp $(SWITCH) $(OUTPUT_DIR)/
	cp $(IMAGES)/*dtb $(OUTPUT_DIR)/images/
	cp $(IMAGES)/*img $(OUTPUT_DIR)/images/
	cp $(IMAGES)/uImage $(OUTPUT_DIR)/images/
	cp $(IMAGES)/*uboot $(OUTPUT_DIR)/images/
	cp Makefile.archive $(OUTPUT_DIR)/Makefile
	cp start_qemu.sh.archive $(OUTPUT_DIR)/start_qemu.sh
	tar -zcvf $(OUTPUT_TGZ).tgz $(OUTPUT_DIR) --transform s/$(OUTPUT_DIR)/$(OUTPUT_TGZ)/

clean:
	rm -rf $(OUTPUT_DIR) $(OUTPUT_TGZ).tgz
