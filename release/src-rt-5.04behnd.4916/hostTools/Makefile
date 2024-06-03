
TOPDIR  := $(shell /bin/pwd)

default:

$(MAKECMDGOALS):
	-cp prebuilt/mksquashfs ./mksquashfs
	mkdir -p mtd-utils-2.0.2
	ln -sf mtd-utils-2.0.2 mtd-utils
	-cp prebuilt/mkfs.ubifs mtd-utils/
	-cp prebuilt/ubinize mtd-utils/
	mkdir -p local_install
	-cp prebuilt/lzop local_install/
	-cp prebuilt/cmplzma ./cmplzma
	-cp prebuilt/lzmacmd ./lzmacmd
	mkdir -p prelink/src
	-cp prebuilt/execstack ./prelink/src/execstack
	-mkdir -p libelf/lib
	-cp prebuilt/libelf.so ./libelf/lib/libelf.so
	rm -f libelf/lib/libelf.so.0
	-(cd libelf/lib && ln -sf libelf.so libelf.so.0)


build_execstack: $(TOPDIR)/prelink/src/execstack


