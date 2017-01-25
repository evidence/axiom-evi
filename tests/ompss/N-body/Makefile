
DIRS=serial ompss 

RECURSIVE=all-recursive clean-recursive check-recursive

all: all-recursive
clean: clean-recursive
check: check-recursive

$(RECURSIVE): $(DIRS)
	@failcom='exit 1';\
        target=`echo $@ | sed s/-recursive//`; \
        for subdir in $(DIRS); do \
                (cd $$subdir && $(MAKE) $$target) \
          || eval $$failcom; \
        done;
