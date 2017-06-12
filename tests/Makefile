
export P

.PHONY: clean distclean build install

SUBDIRS=axiom gasnet ompss

install build clean distclean:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir $@; done

.PHONY: start stop run target sync

start stop run target sync:
	$(MAKE) -C ../scripts $@
