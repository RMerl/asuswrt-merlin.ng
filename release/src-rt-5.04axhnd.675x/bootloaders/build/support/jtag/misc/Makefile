CROSS_COMPILE?=/opt/toolchains/crosstools-aarch64-gcc-9.2-linux-4.19-glibc-2.30-binutils-2.32/usr/bin/aarch64-linux

all:
	$(CROSS_COMPILE)-gcc -c armv8_disable_mmu.S -o armv8_disable_mmu.o -O0
	$(CROSS_COMPILE)-objcopy -O binary --only-section=.text armv8_disable_mmu.o armv8_disable_mmu.bin
	@ret=0x`$(CROSS_COMPILE)-objdump -d armv8_disable_mmu.o | grep ret | perl -pe "s/\s*(\w+):.*/\1/"` ;\
	 echo 'restore armv8_disable_mmu.bin binary $$pc+0x1000' > mmu_disable.gdb ; \
	 echo 'set $$pc=$$pc+0x1000' >> mmu_disable.gdb ;  \
	 echo 'b $$pc+'$$ret >> mmu_disable.gdb ; \
	 echo 'c' >> mmu_disable.gdb ;

clean:
	rm *.o


.PHONY:63138 63148

$(BRCM_CHIP):
	-$(MAKE) -C ddrinit $(BRCM_CHIP)
