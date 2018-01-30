
.PHONY: clean distclean build install mrproper

SUBDIRS=axiom gasnet ompss

install build clean distclean mrproper:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir $@; done
