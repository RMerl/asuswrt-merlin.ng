ifeq ($(strip $(CONFIG_BRCM_QEMU)),)
dtb-$(CONFIG_OF) += \
	9$(CONFIG_BCM_CHIP_NUMBER).dtb
else
dtb-$(CONFIG_OF) += \
	9$(CONFIG_BCM_CHIP_NUMBER)_qemu.dtb
endif

include $(DTS_DIR)/dts/make.dtbs
