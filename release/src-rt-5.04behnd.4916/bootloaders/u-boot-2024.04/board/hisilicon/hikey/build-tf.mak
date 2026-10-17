CROSS_COMPILE	:= aarch64-linux-gnu-
output_dir	:= $(PWD)/../bin
makejobs	:= $(nproc)
makethreads	:= $(shell dc -e "$(makejobs) 1 + p")
make_options	:= GCC49_AARCH64_PREFIX=$CROSS_COMPILE \
		-j$(makethreads) -l$(makejobs)

BL30_HIKEY	:= $(output_dir)/mcuimage.bin
BL33_HIKEY	:= $(output_dir)/u-boot-hikey.bin

.PHONY: help
help:
	@echo "****  Common Makefile  ****"
	@echo "example:"
	@echo "make -f build-tf.mak build"

.PHONY: have-crosscompiler
have-crosscompiler:
	@echo -n "Check that $(CROSS_COMPILE)gcc is available..."
	@which $(CROSS_COMPILE)gcc > /dev/null ; \
	if [ ! $$? -eq 0 ] ; then \
	   echo "ERROR: cross-compiler $(CROSS_COMPILE)gcc not in PATH=$$PATH!" ; \
	   echo "ABORTING." ; \
	   exit 1 ; \
	else \
	   echo "OK" ;\
	fi

build: have-crosscompiler FORCE
	@echo "Build TF for Hikey..."
	rm -rf build/
	CROSS_COMPILE=$(CROSS_COMPILE) \
	make all fip \
	BL30=$(BL30_HIKEY) \
	BL33=$(BL33_HIKEY) \
	DEBUG=1 \
	PLAT=hikey
	@echo "Copy resulting binaries..."
	cp build/hikey/debug/bl1.bin $(output_dir)/bl1-hikey.bin
	cp build/hikey/debug/fip.bin $(output_dir)/fip-hikey.bin

FORCE:
