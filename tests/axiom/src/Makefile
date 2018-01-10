.PHONY: clean build install distclean mrproper install-real

include ../../common.mk

SOURCES:=$(wildcard *.c) $(wildcard dsetools/std-751/*.c) 
OBJS:=$(SOURCES:.c=.o)
DEPS:=$(OBJS:.o=.d)
EXECS:=$(OBJS:.o=)

CFLAGS += -g -O3 -finline-functions -fomit-frame-pointer -Wall -std=gnu11
LDFLAGS += -pthread

build: $(OBJS) $(EXECS)

clean distclean mrproper:
	rm -f $(OBJS) $(DEPS) $(EXECS)

install: build
	mkdir -p $(DESTDIR)/opt/axiom/tests_axiom
	cp $(EXECS) $(DESTDIR)/opt/axiom/tests_axiom



