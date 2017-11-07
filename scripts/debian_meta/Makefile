.PHONY: build install

ifndef DESTDIR
DESTDIR:=
endif

build:

install: build
	mkdir -p $(DESTDIR)/usr
	cp -r sbin $(DESTDIR)/usr
	cp -r etc $(DESTDIR)/
	cp -r lib $(DESTDIR)/
