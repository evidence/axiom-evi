
.PHONY: clean build install distclean

install build clean distclean:
	@$(MAKE) -f makefile-gasnet.mk GASNET:=gasnet-debug $@
	@$(MAKE) -f makefile-gasnet.mk GASNET:=gasnet-performance $@
