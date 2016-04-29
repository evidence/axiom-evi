DTC := dtc
DTB := zynqmp-qemu-arm.dtb

CLEANFILES := *.dtb

.PHONY: all clean

all: $(DTB)

%.dtb : %.dts
	$(DTC) -q -I dts -O dtb $< -o $@

clean:
	rm $(CLEANFILES)
