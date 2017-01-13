
#
# directories
#

COMFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUTPUT_DIR := ${COMFILE_DIR}/../output
TARGET_DIR := $(realpath ${OUTPUT_DIR}/target)
SYSROOT_DIR := $(realpath ${OUTPUT_DIR}/staging)
HOST_DIR := $(realpath ${OUTPUT_DIR}/host)

#
# pkg-config configuration
#

#PKG-CONFIG_SILENCE:=--silence-errors

PKG-CONFIG := PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1 PKG_CONFIG_ALLOW_SYSTEM_LIBS=1 PKG_CONFIG_DIR= PKG_CONFIG_LIBDIR=$(SYSROOT_DIR)/usr/lib/pkgconfig:$(SYSROOT_DIR)/usr/share/pkgconfig PKG_CONFIG_SYSROOT_DIR=$(SYSROOT_DIR) pkg-config $(PKG-CONFIG_SILENCE)
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

#
#
#

ARCH:=aarch64-buildroot-linux-gnu

BINDIR:=$(HOST_DIR)/usr/bin
MCC:=PATH=$(PATH):$(BINDIR) $(ARCH)-mcc
CC:=PATH=$(PATH):$(BINDIR) $(ARCH)-gcc

#
# source file dependencies management
#

DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td

%.o: %.c
%.o: %.c %.d
	$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
	mv -f $*.Td $*.d

%.d: ;
.PRECIOUS: %.d
