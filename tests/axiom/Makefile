
include ../common.mk
include ../../scripts/locations.mk

.PHONY: build install clean distclean mrproper install-real

GUESTDIR := $(TESTS_DIR)/utils/guest
LAUNCHERS := $(GUESTDIR)/run_test_axiom.sh
LAUNCHERS += $(GUESTDIR)/run_suite.sh

build install clean distclean mrproper:
	$(MAKE) -C src $@
	$(MAKE) -C $(APPS_DIR)/tests $@
	if [ "$@"="install" ]; then $(MAKE) install-real; fi

install-real:
	mkdir -p $(DESTDIR)/opt/axiom/tests_axiom
	cp $(LAUNCHERS)  $(DESTDIR)/opt/axiom/tests_axiom
	cp suite/*  $(DESTDIR)/opt/axiom/tests_axiom

.PHONY: tests tests-core

DATE := $(shell date +%Y%m%d_%H%M%S)

tests: tests-core

# DOES NOT WORK!
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
	@echo "distclean  clean the tests"
	@echo "mrproper   clean the tests"
	@echo ""
	@echo "tests      run all the tests"
	@echo "tests-core run core tests (DOES NOT WORK!)"
	@echo ""
