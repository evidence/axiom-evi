
export KERN
export FS
export DISABLE_INSTR

include ../../scripts/params.mk

KERN:=$(_KERN)
FS:=$(_FS)
DISABLE_INSTR:=$(_DISABLE_INSTR)

include ../../scripts/locations.mk

.PHONY: build install clean distclean install-real

GUESTDIR := $(TESTS_DIR)/utils/guest
LAUNCHERS := $(GUESTDIR)/run_test_axiom.sh
LAUNCHERS += $(GUESTDIR)/run_suite.sh

FAKEROOT :=
ifeq ($(FS),seco)
ifndef FAKEROOTKEY
FAKEROOT := fakeroot -i $(ROOTFS).faked -s $(ROOTFS).faked
endif
endif

install:
	$(MAKE) -C $(EVI_APPS_DIR)/tests $@
	$(FAKEROOT) $(MAKE) install-real

ifeq ($(FS),x86)
SUDO := sudo
else
SUDO :=
endif

# default (if not already defined)
DESTDIR:=$(TARGET_DIR)

install-real:
	$(SUDO) mkdir -p $(DESTDIR)/opt/axiom/tests_axiom
	$(SUDO) cp $(LAUNCHERS)  $(DESTDIR)/opt/axiom/tests_axiom
	$(SUDO) cp suite/*  $(DESTDIR)/opt/axiom/tests_axiom

build clean distclean:
	$(MAKE) -C $(EVI_APPS_DIR)/tests $@

.PHONY: start run stop sync target log

start run stop sync target log:
	$(MAKE) -C $(SCRIPTS_DIR) $@

.PHONY: tests tests-core

DATE := $(shell date +%Y%m%d_%H%M%S)

tests: tests-core

tests-core:
	@sshpass -p root \
	  ssh  -q -o StrictHostKeyChecking=no -p 22200 -l root 127.0.0.1 \
	  "/bin/sh -c 'cd /root/tests_axiom; ./run_suite.sh axiom suite_$(subst tests-,,$@)'" 2>&1 | tee tests_$(DATE)_$(subst tests-,,$@).log
	@echo " ";\
	  echo " ";\
	  echo "The results has been saved to tests_$(DATE)_$(subst tests-,,$@).log";\
	  echo " "

.PHONY: help

help:
	@echo ""
	@echo "makefile relevant targets:"
	@echo ""
	@echo "build      build all the tests"
	@echo "install    build and install all the tests"
	@echo "clean      clean the tests"
	@echo ""
	@echo "start      start qemu/switch system"
	@echo "stop       stop qemu/switch"
	@echo "sync       sync running qemu axiom nodes with root filesystem"
	@echo "target     rebuild the root filesystem"
	@echo ""
	@echo "tests      run all the tests"
	@echo "tests-core run core tests"
	@echo ""

