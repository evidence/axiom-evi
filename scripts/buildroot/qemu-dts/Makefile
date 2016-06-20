DTC := dtc
DTB := zynqmp-qemu-arm.dtb zcu102-arm.dtb
AXIOM-DTB := axiom-board.dtb

CLEANFILES := *.dtb

.PHONY: all clean

all: $(AXIOM-DTB)

%.dtb : %.dts
	$(DTC) -q -I dts -O dtb $< -o $@

$(AXIOM-DTB): zcu102-arm.dtb
	cp $< $@

clean:
	-rm $(CLEANFILES)
