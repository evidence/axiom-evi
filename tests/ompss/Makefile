
include ../common.mk

CFLAGS := -O3 -finline-functions -fomit-frame-pointer -Wall -std=gnu11 -fPIC
LDFLAGS :=
LDLIBS :=

OMPSS_CFLAGS := $(CFLAGS) $(call PKG-CFLAGS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)
OMPSS_LDFLAGS := $(LDFLAGS) $(call PKG-LDFLAGS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)
OMPSS_LDLIBS := $(LDLIBS) $(call PKG-LDLIBS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)

MCCFLAGS= --cluster --ompss

SOURCES=$(wildcard src/ompss_*.c)
OBJS=$(SOURCES:.c=.o)
EXECS=$(OBJS:.o=)

DESTDIR=$(TARGET_DIR)/root/tests_ompss
LAUNCHER=$(COMFILE_DIR)/utils/guest/run_tests_OMPSS.sh

%: %.o
	$(MCC) $(MCCFLAGS) $(OMPSS_LDFLAGS) -o $@ $? $(OMPSS_LDLIBS)

%.o: %.c
	$(MCC) $(MCCFLAGS) $(OMPSS_CFLAGS) -c -o $@ $?

.PHONY: clean build install distclean

install: build
	mkdir -p $(DESTDIR)
	-cp $(EXECS) $(DESTDIR)
	-cp $(LAUNCHER) $(DESTDIR)

build: $(OBJS) $(EXECS)

clean distclean:
	rm -f $(OBJS) $(EXECS)
