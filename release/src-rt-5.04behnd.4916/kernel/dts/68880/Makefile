9$(CONFIG_BCM_CHIP_NUMBER).dtb := $(shell cp -f $(KERNEL_DIR)/include/generated/autoconf.h $(DTB_DIR)/inc/linux_autoconf.h)

# Enables support for device-tree overlays
DTC_FLAGS := -@

dtb-y := $(patsubst $(src)/%.dts,%.dtb, $(wildcard $(src)/*.dts))
always  :=  $(dtb-y)

.PHONY:dtbs_clean
dtbs_clean:
	rm -vf *.dtb .*.tmp .*.cmd inc/linux_autoconf.h
