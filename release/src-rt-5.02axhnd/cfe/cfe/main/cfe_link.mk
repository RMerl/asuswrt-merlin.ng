#
# This Makefile snippet takes care of linking the firmware.
#

pci : $(PCICOMMON) $(PCIMACHDEP)
	echo done

cfe cfe.bin : $(CRT0OBJS) $(BSPOBJS) $(LIBCFE)
	$(GLD) -o cfe -Map cfe.map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -lcfe $(LDLIBS)
	$(OBJDUMP) -d cfe > cfe.dis
	$(OBJCOPY) --output-target=binary cfe cfe.bin
	$(OBJCOPY) --input-target=binary --output-target=srec cfe.bin cfe.srec

cfe.flash : cfe.bin mkflashimage
	./mkflashimage -v ${ENDIAN} -B ${CFG_BOARDNAME} -V ${CFE_VER_MAJ}.${CFE_VER_MIN}.${CFE_VER_ECO} cfe.bin cfe.flash
	$(OBJCOPY) --input-target=binary --output-target=srec cfe.flash cfe.flash.srec


clean :
	rm -f *.o *~ cfe cfe.bin cfe.dis cfe.map cfe.srec
	rm -f makereg ${CPU}_socregs.inc mkpcidb pcidevs_data2.h mkflashimage
	rm -f build_date.c
	rm -f libcfe.a
	rm -f cfe.flash cfe.flash.srec $(CLEANOBJS)

distclean : clean
