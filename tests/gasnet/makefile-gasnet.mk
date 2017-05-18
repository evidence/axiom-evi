
export P

include ../common.mk

# gasnet conduit selection

GASNET:=gasnet-performance
GASNET_CONDUIT:=axiom
GASNET_MODE:=par

# = per | dbg
GASNET_TYPE:=$(subst debug,dbg,$(subst performance,per,$(subst gasnet-,,$(GASNET))))

# my files

SOURCES=$(wildcard src/gasnet_*.c)
OBJS=$(SOURCES:.c=_$(GASNET_CONDUIT)_$(GASNET_TYPE).o)
DEPS=$(OBJS:.o=.d)
EXECS=$(OBJS:.o=)

# my cflags,ldflags and ldlibs

CFLAGS := -finline-functions -fomit-frame-pointer -Wall -std=gnu11 -fPIC
CFLAGS += $(call PKG-CFLAGS,axiom_user_api axiom_init_api axiom_run_api axiom_allocator evi_lmm)
LDFLAGS := $(call PKG-LDFLAGS,axiom_user_api axiom_init_api axiom_run_api axiom_allocator evi_lmm)
LDLIBS := $(call PKG-LDLIBS,axiom_user_api axiom_init_api axiom_run_api axiom_allocator evi_lmm) -pthread

# gasnet fragment and hack

include $(SYSROOT_DIR)/$(shell $(PKG-CONFIG) --variable=includedir $(GASNET))/$(GASNET_CONDUIT)-conduit/$(GASNET_CONDUIT)-$(GASNET_MODE).mak

GASNET_PREFIX=${SYSROOT_DIR}/usr
GASNET_LDFLAGS+=-L$(SYSROOT_DIR)/$(shell $(PKG-CONFIG) --variable=libdir $(GASNET))/..
GASNET_CFLAGS+=-I$(SYSROOT_DIR)/$(shell $(PKG-CONFIG) --variable=includedir $(GASNET))/../axiom
GASNET_CFLAGS+= -Wno-unused-function 
ifeq ($(GASNET_CONDUIT),udp)
GASNET_LDFLAGS+=-lstdc++
endif

# link rules

%: %.o
	$(CC) $(LDFLAGS) $(GASNET_LDFLAGS) -o $@ $? $(LDLIBS) $(GASNET_LIBS) 

# compile rules

DEPFLAGS = -MT $@ -MMD -MP -MF $@.Td

%_$(GASNET_CONDUIT)_$(GASNET_TYPE).d: ;

%_$(GASNET_CONDUIT)_$(GASNET_TYPE).o: %.c %.d
	$(CC) $(DEPFLAGS) $(CFLAGS) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) -c -o $@ $<
	mv -f $@.Td $*_$(GASNET_CONDUIT)_$(GASNET_TYPE).d

# my targets

DESTDIR:=$(TARGET_DIR)/root/tests_gasnet_$(GASNET_TYPE)
LAUNCHERS:=$(COMFILE_DIR)/utils/guest/run_test_gasnet.sh
LAUNCHERS+=$(COMFILE_DIR)/utils/guest/run_suite.sh

.PHONY: clean build install distclean install-real

install: build
	$(FAKEROOT) $(MAKE) -f makefile-gasnet.mk install-real

install-real:
	mkdir -p $(DESTDIR)
	for EXE in $(EXECS); do cp $${EXE} $(DESTDIR)/`echo $${EXE}|sed -e 's,.*/,,' -e 's,_$(GASNET_TYPE),,'`; done
	cp $(LAUNCHERS) $(DESTDIR)
	cp suite/* $(DESTDIR)

-include $(DEPS)

build: $(EXECS) $(OBJS)

clean distclean:
	-rm -f $(OBJS) $(DEPS) $(EXECS)
