export P
export E
export S

_ccred:="\033[0;31m"
ccred:=$(shell echo -e $(_ccred))
ccgreen:=$(shell echo -e "\033[0;32m")
ccblue:=$(shell echo -e "\033[0;34m")
_ccend:="\033[0m"
ccend:=$(shell echo -e $(_ccend))

define ENV_HELP =
$(ccred)Command line enviroment variables not defined!$(ccend)
You must define some variable on the command line (for example "make P=1 E=0 run"):
* P ($(ccgreen)required$(ccend))
    0: use buildroot environment 
    1: use Petalinux/Linaro (with SECO rootfs) environment
* E ($(ccgreen)optional$(ccend), used only if P=1)
    0: use buildroot kernel for QEMU (default if non specified)
    1: use bsp kernel for QEMU (actually does not work)
* S ($(ccgreen)optional$(ccend), used only during qemu run)
    0: the rootfs in on a static qcow2 (one for every machine)
    1: the rootfs in on a temp qcow2 snapshot (default)
endef

ifndef P
$(error $(ENV_HELP))
endif

# P=1 => Petalinux mode (compile NOT using buildroot but petalinux)
# E=1 => if P=1 then Petalinux with emulation mode (as P=1 but NOT use the AxiomBSP kernel)
# S=1 => use a temporally qemu snapshot

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

QEMU_PARAM := "--longenq"
ifeq ($(LONG_ENQUEUE), 0)
QEMU_PARAM := ""
endif
ifeq ($(P),1)
QEMU_PARAM += "--petalinux"
endif
# default: usa a qemu temp snapshot image
ifndef S
S:=1
endif
ifeq ($(S),1)
QEMU_PARAM += "--snapshot"
endif


IDS := $(shell seq 0 $$(( $(VMS) - 1 )))
RUNS := $(addprefix run,$(IDS))


MASTERROOTFS_SECO_NAME := zynq-rootfs_seco
MASTERROOTFS_BR_NAME := zynq-rootfs_br
ifeq ($(P),1)
MASTERROOTFS_NAME := $(MASTERROOTFS_SECO_NAME)
else
MASTERROOTFS_NAME := $(MASTERROOTFS_BR_NAME)
endif


.PHONY: $(RUNS) run switch stop clean clean-log

run: clean-log stop switch $(RUNS)

$(RUNS): run%:
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

clean-log:
	-@rm $(LOG_FILE)

clean: clean-log
