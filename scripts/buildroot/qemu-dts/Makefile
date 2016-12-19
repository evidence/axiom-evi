DTC := dtc
DTB := zynqmp-qemu-arm.dtb zcu102-arm.dtb
AXIOM-DTB := axiom-board.dtb
AXIOM-DTB-LONG := axiom-board-long-enqueue.dtb

CLEANFILES := *.dtb

.PHONY: all clean

all: $(AXIOM-DTB) $(AXIOM-DTB-LONG)

%.dtb : %.dts
	$(DTC) -q -I dts -O dtb $< -o $@

$(AXIOM-DTB): zcu102-arm.dtb
	cp $< $@

$(AXIOM-DTB-LONG): zcu102-arm-long-enqueue.dtb
	cp $< $@
clean:
	-rm $(CLEANFILES)
