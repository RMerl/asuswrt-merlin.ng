9$(CONFIG_BCM_CHIP_NUMBER).dtb := $(shell cp -f $(KERNEL_DIR)/include/generated/autoconf.h $(DTB_DIR)/inc/linux_autoconf.h)

dtb-y    := $(patsubst $(src)/%.dts,%.dtb, $(wildcard $(src)/*.dts))

always	:=  $(dtb-y)

.PHONY:dtsb_clean
dtbs_clean:
	rm -vf *.dtb inc/linux_autoconf.h
