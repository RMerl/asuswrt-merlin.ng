
default:

$(MAKECMDGOALS):
	cp prebuilt/mksquashfs ./mksquashfs
	mkdir -p mtd-utils-2.0.1/
	cp prebuilt/mkfs.ubifs mtd-utils-2.0.1/
	cp prebuilt/ubinize mtd-utils-2.0.1/
	mkdir -p local_install
	cp prebuilt/$(BUILD_NAME)/lzop local_install/
	cp prebuilt/cmplzma ./cmplzma
	cp prebuilt/lzmacmd ./lzmacmd


