
export P

include ../../scripts/locations.mk

.PHONY: clean build install distclean

install: install-int install-ext
build: build-int build-ext
clean: clean-int clean-ext
distclean: distclean-int distclean-ext

FAKEROOT :=
ifeq ($(P),1)
ifndef FAKEROOTKEY
FAKEROOT := fakeroot -i $(ROOTFS).faked -s $(ROOTFS).faked
endif
endif

#
# external tests
#

.PHONY: install-ext build-ext clean-ext distclean-ext install-ext-real

install-ext: build-ext
	$(FAKEROOT) $(MAKE) install-ext-real

install-ext-real:
	for DESTDIR in $(TARGET_DIR); do \
		mkdir -p $${DESTDIR}/root/tests_gasnet_dbg ;\
		cp $(GASNET_BUILD_DIR_DEBUG)/axiom-conduit/test* \
			$${DESTDIR}/root/tests_gasnet_dbg ;\
		mkdir -p $${DESTDIR}/root/tests_gasnet_per ;\
		cp $(GASNET_BUILD_DIR_PERF)/axiom-conduit/test* \
			$${DESTDIR}/root/tests_gasnet_per ;\
	done

build-ext:
	@$(MAKE) -j$(BUILDCPUS) -C $(GASNET_BUILD_DIR_DEBUG) tests-par
	@$(MAKE) -j$(BUILDCPUS) -C $(GASNET_BUILD_DIR_PERF) tests-par


clean-ext distclean-ext:
	-rm -f "$(GASNET_BUILD_DIR_DEBUG)/axiom-conduit/test*"
	-rm -f "$(GASNET_BUILD_DIR_PERF)/axiom-conduit/test*"

#
# own tests
#

.PHONY: install-int build-int clean-int distclean-int

install-int build-int clean-int distclean-int:
	@$(MAKE) -f makefile-gasnet.mk GASNET:=gasnet-debug $(subst -int,,$@)
	@$(MAKE) -f makefile-gasnet.mk GASNET:=gasnet-performance $(subst -int,,$@)

#
# utility
#

.PHONY: start run stop sync target log gasnet

start run stop sync target log gasnet:
	$(MAKE) -C $(SCRIPTS_DIR) $@

#
# tests
#

.PHONY: tests tests-core tests-exit tests-hsl tests-own tests-perf

DATE := $(shell date +%Y%m%d_%H%M%S)

tests: tests-core tests-exit tests-hsl tests-own tests-perf

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
	@echo "start      start qemu/switch system"
	@echo "stop       stop qemu/switch"
	@echo "sync       copy target root filesystem on running vms"
	@echo ""
	@echo "tests      run all the tests"
	@echo "tests-core run core tests"
	@echo "tests-exit run exit tests"
	@echo "tests-hsl  run high speed lock tests"
	@echo "tests-own  run own tests"
	@echo "tests-perf run performance tests"
	@echo ""
