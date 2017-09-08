
export KERN
export FS
export DISABLE_INSTR

include ../../scripts/params.mk

KERN:=$(_KERN)
FS:=$(_FS)
DISABLE_INSTR:=$(_DISABLE_INSTR)

include ../common.mk
include ../../scripts/locations.mk

CFLAGS := -g -finline-functions -fomit-frame-pointer -Wall -Wno-unused -std=gnu11 -fPIC
LDFLAGS :=
LDLIBS := -lm

ifeq ($(FS),seco)
EXTRA_OMPSS_CFLAGS := -I$(ROOTFS)/usr/include/aarch64-linux-gnu
else
EXTRA_OMPSS_CFLAGS :=
endif

OMPSS_CFLAGS := $(CFLAGS) $(EXTRA_OMPSS_CFLAGS) $(call PKG-CFLAGS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)
OMPSS_LDFLAGS := $(LDFLAGS) $(call PKG-LDFLAGS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)
OMPSS_LDLIBS := $(LDLIBS) $(call PKG-LDLIBS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)

ifneq ($(FS),br)
MCCFLAGS= --cluster --ompss -k
else
MCCFLAGS= --cluster --ompss --instrument -k
endif

SOURCES:=$(wildcard src/ompss_*.c)
OBJS:=$(SOURCES:.c=.o)
EXECS:=$(OBJS:.o=)

DESTDIR:=$(TARGET_DIR)/root/tests_ompss
LAUNCHER:=$(COMFILE_DIR)/utils/guest/run_test_ompss.sh
LAUNCHER+=$(COMFILE_DIR)/utils/guest/run_suite.sh
LAUNCHER+=$(COMFILE_DIR)/utils/guest/run_test_nbody.sh
LAUNCHER+=$(COMFILE_DIR)/utils/guest/run_test_nbody_extrae.sh
LAUNCHER+=$(COMFILE_DIR)/utils/guest/run_test_ompss_extrae.sh
LAUNCHER+=$(COMFILE_DIR)/utils/guest/extrae.xml

%: %.o
	$(MCC) $(MCCFLAGS) $(OMPSS_LDFLAGS) -o $@ $? $(OMPSS_LDLIBS)

%.o: %.c
	$(MCC) $(MCCFLAGS) $(OMPSS_CFLAGS) -c -o $@ $?

#
# help
#

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
	@echo "tests-own  run own tests"
	@echo ""

#
# build/install/clean/distclean
#

ifdef FROMDEB
SUDO :=
endif

.PHONY: clean build install distclean

install: build install-nbody
	$(FAKEROOT) $(SUDO) mkdir -p $(DESTDIR)
	-$(FAKEROOT) $(SUDO) cp $(EXECS) $(DESTDIR)
	-$(FAKEROOT) $(SUDO) cp $(LAUNCHER) $(DESTDIR)
	-$(FAKEROOT) $(SUDO) cp suite/* $(DESTDIR)

build: $(OBJS) $(EXECS) build-nbody

clean distclean: clean-nbody
	rm -f $(OBJS) $(EXECS)

#
# NBODY
#

NBODY_DIR:=N-body/ompss
NBODY_SOURCES=$(wildcard $(NBODY_DIR)/*.c)
NBODY_OBJS=$(NBODY_SOURCES:.c=.o)
NBODY_EXEC=$(NBODY_DIR)/nbody

.PHONY: install-nbody build-nbody clean-nbody distclean-nbody

install-nbody: build-nbody
	$(FAKEROOT) $(SUDO) mkdir -p $(DESTDIR)
	$(FAKEROOT) $(SUDO) cp -f $(NBODY_EXEC) $(DESTDIR)
	$(FAKEROOT) $(SUDO) cp -f $(NBODY_DIR)/../particles-*.in $(DESTDIR)
	$(FAKEROOT) $(SUDO) cp -f $(NBODY_DIR)/../particles-*.ref $(DESTDIR)

build-nbody: $(NBODY_EXEC)

$(NBODY_EXEC): $(NBODY_OBJS)


clean-nbody distclean-nbody:
	rm -f $(NBODY_OBJS) $(NBODY_EXEC)


#
# start/stop/sync/target/log
#

.PHONY: start run stop sync target log

start run stop sync target log:
	$(MAKE) -C $(SCRIPTS_DIR) $@

#
# tests
#

.PHONY: tests tests-own

DATE := $(shell date +%Y%m%d_%H%M%S)

tests: tests-own

tests-own:
	@sshpass -p root \
	  ssh  -q -o StrictHostKeyChecking=no -p 22200 -l root 127.0.0.1 \
	  "/bin/sh -c 'cd /root/tests_ompss; ./run_suite.sh ompss suite_$(subst tests-,,$@)'" 2>&1 | tee tests_$(DATE)_$(subst tests-,,$@).log
	@echo " ";\
	  echo " ";\
	  echo "The results has been saved to tests_$(DATE)_$(subst tests-,,$@).log";\
	  echo " "
