
include ../common.mk
include ../../scripts/locations.mk

#
# main targets
#

.PHONY: clean build install distclean mr-proper

ifeq ($(MODE),x86)

_ccred:="\033[0;31m"
ccred:=$(shell echo -e $(_ccred))
ccgreen:=$(shell echo -e "\033[0;32m")
ccblue:=$(shell echo -e "\033[0;34m")
_ccend:="\033[0m"
ccend:=$(shell echo -e $(_ccend))

build install clean distclean mrproper:
	@echo "$(ccred)GASNET tests will not be build on X86 NATIVE$(ccend)"

else

build: build-int build-ext
install: install-int install-ext
clean: clean-int clean-ext
distclean: distclean-int distclean-ext
mrproper: mrproper-int mrproper-ext

endif

#
# external tests
#

.PHONY: install-ext build-ext clean-ext distclean-ext install-ext-real

install-ext:
	TYPE=dbg ;\
	  MYTARGETDIR=$(DESTDIR)/opt/axiom/tests_gasnet_$${TYPE} ;\
	  mkdir -p $${MYTARGETDIR} ;\
	  cp $(GASNET_BUILD_DIR_DEBUG)/axiom-conduit/test* $${MYTARGETDIR}
	TYPE=per ;\
	  MYTARGETDIR=$(DESTDIR)/opt/axiom/tests_gasnet_$${TYPE} ;\
	  mkdir -p $${MYTARGETDIR} ;\
	  cp $(GASNET_BUILD_DIR_PERF)/axiom-conduit/test* $${MYTARGETDIR}

build-ext:
	@$(MAKE) -j$(BUILDCPUS) -C $(GASNET_BUILD_DIR_DEBUG) tests-par
	@$(MAKE) -j$(BUILDCPUS) -C $(GASNET_BUILD_DIR_PERF) tests-par


clean-ext distclean-ext:
	-rm -f "$(GASNET_BUILD_DIR_DEBUG)/axiom-conduit/test*"
	-rm -f "$(GASNET_BUILD_DIR_PERF)/axiom-conduit/test*"

#
# own tests
#

.PHONY: install-int build-int clean-int distclean-int mrproper-int

install-int build-int clean-int distclean-int mrproper-int:
	@$(MAKE) \
		-f makefile-gasnet.mk \
		GASNET:=gasnet-debug \
		$(subst -int,,$@)
	@$(MAKE) \
		-f makefile-gasnet.mk \
		GASNET:=gasnet-performance \
		$(subst -int,,$@)

#
# tests
#

.PHONY: tests tests-core tests-exit tests-hsl tests-own tests-perf

DATE := $(shell date +%Y%m%d_%H%M%S)

tests: tests-core tests-exit tests-hsl tests-own tests-perf

# DOES NOT WORK!
tests-core tests-exit tests-hsl tests-own tests-perf:
	-@ssh-keygen -f "$${HOME}/.ssh/known_hosts" -R [127.0.0.1]:22200
	@sshpass -p root \
	  ssh  -q -o StrictHostKeyChecking=no -p 22200 -l root 127.0.0.1 \
	  "/bin/sh -c 'cd /root/tests_gasnet_dbg; ./run_suite.sh gasnet suite_$(subst tests-,,$@)'" 2>&1 | tee tests_$(DATE)_$(subst tests-,,$@).log
	@echo " ";\
	  echo " ";\
	  echo "The results has been saved to tests_$(DATE)_$(subst tests-,,$@).log";\
	  echo " "

#
# help
#

.PHONY: help

help:
	@echo ""
	@echo "makefile relevant targets:"
	@echo ""
	@echo "build      build all the tests"
	@echo "install    build and install all the tests [DEFAULT TARGET]"
	@echo "clean      clean the tests"
	@echo ""
	@echo "gasnet     rebuild gasnet library ('build' does NOT rebuild the library)"
	@echo "target     rebuild target root filesystem"
	@echo ""
	@echo "tests      run all the tests"
	@echo "tests-core run core tests"
	@echo "tests-exit run exit tests"
	@echo "tests-hsl  run high speed lock tests"
	@echo "tests-own  run own tests"
	@echo "tests-perf run performance tests"
	@echo ""
