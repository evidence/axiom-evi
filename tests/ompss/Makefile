
.PHONY: clean build install

MKFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SYSROOT_DIR := $(MKFILE_DIR)/../../output/staging

PKG-CONFIG := PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1 PKG_CONFIG_ALLOW_SYSTEM_LIBS=1 PKG_CONFIG_DIR= PKG_CONFIG_LIBDIR=$(SYSROOT_DIR)/usr/lib/pkgconfig:$(SYSROOT_DIR)/usr/share/pkgconfig PKG_CONFIG_SYSROOT_DIR=$(SYSROOT_DIR) pkg-config
ifeq ($(shell $(PKG-CONFIG) axiom_user_api >/dev/null || echo fail),fail)
    # GNU make bug 10593: an environment variale can not be inject into sub-shell (only in sub-make)
    # so a default for PKG_CONFIG_PATH can not be set into a makefile (and used by sub-shell)
    $(warning axiom_user_api.pc is not found (PKG_CONFIG_SYSROOT_DIR is '$(SYSROOT_DIR)' $(TARGET_DIR)))
endif

# PKG-CFLAGS equivalent to "pkg-config --cflags ${1}" into the cross sysroot
PKG-CFLAGS = $(shell $(PKG-CONFIG) --cflags ${1})
# PKG-LDFLAGS equivalent to "pkg-config --libs-only-other --libs-only-L ${1}" into the cross sysroot
PKG-LDFLAGS = $(shell $(PKG-CONFIG) --libs-only-other --libs-only-L ${1})
# PKG-LDLIBS equvalent to "pkg-config --libs-only-l ${1}" into the cross sysroot
PKG-LDLIBS = $(shell $(PKG-CONFIG) --libs-only-l ${1})
# PKG-VARIABLE equivalent to 'pkg-config --variable=${1} ${2}' NMB: no space when the varibale name is specified
PKG-VARIABLE = $(shell $(PKG-CONFIG) --variable=${1} ${2})

ARCH=aarch64-buildroot-linux-gnu

BINDIR=$(call PKG-VARIABLE,bindir,mcxx)
MCC=PATH=$(PATH):$(BINDIR) $(ARCH)-mcc

CFLAGS = -O3 -finline-functions -fomit-frame-pointer -Wall -std=gnu11 -fPIC
LDFLAGS =
LDLIBS =

OMPSS_CFLAGS=$(CFLAGS) $(call PKG-CFLAGS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)
OMPSS_LDFLAGS=$(LDFLAGS) $(call PKG-LDFLAGS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)
OMPSS_LDLIBS=$(LDLIBS) $(call PKG-LDLIBS, axiom_user_api axiom_init_api  axiom_run_api axiom_allocator evi_lmm)

MCCFLAGS= --cluster --ompss

SOURCES=$(wildcard src/ompss_*.c)
OBJS=$(SOURCES:.c=.o)
EXECS=$(OBJS:.o=)

DESTDIR=$(MKFILE_DIR)/../../output/target/root/tests_ompss
LAUNCHER=$(MKFILE_DIR)/../utils/guest/run_tests_OMPSS.sh

%: %.o
	$(MCC) $(MCCFLAGS) $(OMPSS_LDFLAGS) -o $@ $? $(OMPSS_LDLIBS)

%.o: %.c
	$(MCC) $(MCCFLAGS) $(OMPSS_CFLAGS) -c -o $@ $?

install: build
	mkdir -p $(DESTDIR)
	-cp $(EXECS) $(DESTDIR)
	-cp $(LAUNCHER) $(DESTDIR)

build: $(OBJS) $(EXECS)

clean:
	rm -f $(OBJS) $(EXECS)
