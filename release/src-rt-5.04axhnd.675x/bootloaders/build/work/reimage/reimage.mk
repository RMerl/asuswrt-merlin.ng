
bcm_uboot_reimage_format ?= split

reimage : uboot
	-cp ../targets/$(PROFILE)/metadata.bin obj/binaries/
	build/work/masq_cfe  \
		--ubootobjdir=obj/$(bcm_uboot_uboot_prefix)uboot/ \
		--payload=$(bcm_uboot_reimage_payload) \
		--blocksize=$(bcm_uboot_reimage_blocksize) \
		--format=$(bcm_uboot_reimage_format) \
		--btrm=$(bcm_uboot_reimage_btrm) \
		--chip=$(BRCM_CHIP)

