
include ../common.mk
include ../../scripts/locations.mk

CFLAGS := -g -finline-functions -fomit-frame-pointer -Wall -Wno-unused -std=gnu11 -fPIC
LDFLAGS := -rdynamic -Wl,-T$(ALLOCATOR_LIB_DIR)/ldscript/aarch64/xs_map64.lds
LDLIBS := -lm

EXTRA_OMPSS_CFLAGS :=

OMPSS_CFLAGS := $(CFLAGS) $(EXTRA_OMPSS_CFLAGS)
OMPSS_LDFLAGS := $(LDFLAGS)
OMPSS_LDLIBS := $(LDLIBS)

MCCFLAGS := --cluster --ompss -k

SOURCES:=$(wildcard src/ompss_*.c)
OBJS:=$(SOURCES:.c=.o) src/ompss_evimm_debug.o
EXECS:=$(OBJS:.o=)

MYDESTDIR:=$(DESTDIR)/opt/axiom/tests_ompss
GUESTDIR := $(TESTS_DIR)/utils/guest
LAUNCHER:=$(GUESTDIR)/run_suite.sh
LAUNCHER+=$(GUESTDIR)/run_test_ompss.sh
LAUNCHER+=$(GUESTDIR)/run_test_nbody.sh
LAUNCHER+=$(GUESTDIR)/run_test_ompss_extrae.sh
LAUNCHER+=$(GUESTDIR)/run_test_nbody_extrae.sh
LAUNCHER+=$(GUESTDIR)/run_test_ompss_sched.sh
LAUNCHER+=$(GUESTDIR)/run_test_nbody_sched.sh
LAUNCHER+=$(GUESTDIR)/extrae.xml

.DEFAULT_GOAL := build

%: %.o
	$(MCC) $(MCCFLAGS) $(OMPSS_LDFLAGS) -o $@ $? $(OMPSS_LDLIBS)

%.o: %.c
	$(MCC) $(MCCFLAGS) $(OMPSS_CFLAGS) -c -o $@ $?

src/ompss_evimm_debug.o: src/ompss_evimm.c
	$(MCC) $(MCCFLAGS) $(OMPSS_CFLAGS) -DMATRIX_DEBUG -c -o $@ $?

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
	@echo "distclean  clean the tests"
	@echo "mrproper   clean the tests"

#
# build/install/clean/distclean
#

.PHONY: clean build install distclean mrproper

install: install-nbody
	mkdir -p $(MYDESTDIR)
	-cp $(EXECS) $(MYDESTDIR)
	-cp $(LAUNCHER) $(MYDESTDIR)
	-cp suite/* $(MYDESTDIR)

build: $(OBJS) $(EXECS) build-nbody

clean distclean mrproper: clean-nbody
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
	mkdir -p $(MYDESTDIR)
	cp -f $(NBODY_EXEC) $(MYDESTDIR)
	cp -f $(NBODY_DIR)/../particles-*.in $(MYDESTDIR)
	cp -f $(NBODY_DIR)/../particles-*.ref $(MYDESTDIR)

build-nbody: $(NBODY_EXEC)

$(NBODY_EXEC): $(NBODY_OBJS)

clean-nbody distclean-nbody mrproper-nbody:
	rm -f $(NBODY_OBJS) $(NBODY_EXEC)


#
# tests
#

.PHONY: tests tests-own

DATE := $(shell date +%Y%m%d_%H%M%S)

tests: tests-own

# DOES NOT WORK!
tests-own:
	@sshpass -p root \
	  ssh  -q -o StrictHostKeyChecking=no -p 22200 -l root 127.0.0.1 \
	  "/bin/sh -c 'cd /root/tests_ompss; ./run_suite.sh ompss suite_$(subst tests-,,$@)'" 2>&1 | tee tests_$(DATE)_$(subst tests-,,$@).log
	@echo " ";\
	  echo " ";\
	  echo "The results has been saved to tests_$(DATE)_$(subst tests-,,$@).log";\
	  echo " "
