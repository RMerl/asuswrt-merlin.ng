# Enables support for device-tree overlays
DTC_FLAGS += -@
BRCM_INCLUDE :=../../bcmdrivers/opensource/include/bcm963xx/

dtbo-y += $(patsubst $(src)/%.dtso,%.dtbo, $(wildcard $(src)/*.dtso))

always-y  += $(dtbo-y)

%.pre.dtso.o: %.dtso
	$(CPP) -nostdinc -x assembler-with-cpp -undef -D__DTS__ -I$(DTC_INCLUDE) -I$(BRCM_INCLUDE) -o $@ $^

%.dtbo: %.pre.dtso.o
	$(DTC) $(DTC_FLAGS) -I dts -O dtb -o $@ $^

.PHONY:dtbs_clean
dtbs_clean:
	rm -vf *.dtb *.dtbo .*.tmp .*.cmd *.o *.S
