
include ../common.mk
include ../../scripts/locations.mk

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

include $(ROOTFSTMP)/$(shell $(PKG-CONFIG) --variable=includedir $(GASNET))/$(GASNET_CONDUIT)-conduit/$(GASNET_CONDUIT)-$(GASNET_MODE).mak

GASNET_PREFIX=${ROOTFSTMP}/usr
GASNET_LDFLAGS+=-L$(ROOTFSTMP)/$(shell $(PKG-CONFIG) --variable=libdir $(GASNET))/..
GASNET_CFLAGS+=-I$(ROOTFSTMP)/$(shell $(PKG-CONFIG) --variable=includedir $(GASNET))/../axiom
GASNET_CFLAGS+= -Wno-unused-function 
ifeq ($(GASNET_CONDUIT),udp)
GASNET_LDFLAGS+=-lstdc++
endif

ifeq ($(MODE),x86)
GASNET_CFLAGS+=-I$(ROOTFSTMP)/$(shell $(PKG-CONFIG) --variable=includedir $(GASNET))
GASNET_CFLAGS+=-I$(ROOTFSTMP)/$(shell $(PKG-CONFIG) --variable=includedir $(GASNET))/axiom-conduit
GASNET_CFLAGS+=$(call PKG-CFLAGS, $(GASNET))
endif

# link rules

%: %.o

	@echo MY LDFL=$(LDFLAGS)
	@echo MY_LFLIBS=$(LDLIBS)

	$(CC) $(LDFLAGS) $(GASNET_LDFLAGS) -o $@ $? $(LDLIBS) $(GASNET_LIBS) 

# compile rules

DEPFLAGS = -MT $@ -MMD -MP -MF $@.Td

%_$(GASNET_CONDUIT)_$(GASNET_TYPE).d: ;

%_$(GASNET_CONDUIT)_$(GASNET_TYPE).o: %.c %.d
	$(CC) $(DEPFLAGS) $(CFLAGS) $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) -c -o $@ $<
	mv -f $@.Td $*_$(GASNET_CONDUIT)_$(GASNET_TYPE).d

# my targets

MYTARGETDIR:=$(DESTDIR)/opt/axiom/tests_gasnet_$(GASNET_TYPE)
GUESTDIR:=$(TESTS_DIR)/utils/guest
LAUNCHERS:=$(GUESTDIR)/run_test_gasnet.sh
LAUNCHERS+=$(GUESTDIR)/run_suite.sh

.PHONY: clean build install distclean install-real mrproper

install:
	-mkdir -p $(MYTARGETDIR)
	for EXE in $(EXECS); do \
	  cp $${EXE} $(MYTARGETDIR)/`echo $${EXE}|sed -e 's,.*/,,' -e 's,_$(GASNET_TYPE),,'`;\
	done
	cp $(LAUNCHERS) $(MYTARGETDIR)
	cp suite/* $(MYTARGETDIR)

-include $(DEPS)

build: $(EXECS) $(OBJS)

clean distclean mrproper:
	-rm -f $(OBJS) $(DEPS) $(EXECS)
