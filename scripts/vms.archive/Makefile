# number of Virtual Machines to run
VMS := 2

IMAGES := ./images
QEMU := ./qemu-system-aarch64
SWITCH := ./axiom_switch
SWITCH_PARAM := -r -n $(VMS)
LOG_FILE := make.log

ifdef SWITCH
SWITCH_RUN = $(SWITCH) $(SWITCH_PARAM) >> $(LOG_FILE) 2>&1 &
else
SWITCH_RUN=
SWITCH_PARAM=
endif

IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))



.PHONY: $(RUNS) run switch stop clean clean-log

run: clean-log stop switch $(RUNS)

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

clean: clean-log
