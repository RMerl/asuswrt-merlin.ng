# Makefile for the bcm963xx board drivers
#


obj-y := board.o bcm63xx_flash.o  \
	board_wl.o board_proc.o board_image.o board_util.o \
	board_ioctl.o
ifeq ($(strip $(CONFIG_DT_SUPPORT_ONLY)),)
    obj-y +=bcm63xx_led.o bcm63xx_gpio.o pushbutton.o
endif

ifeq ($(strip $(CONFIG_BRCM_QEMU)),)
    obj-y +=board_dg.o
endif

obj-$(CONFIG_BCM_ROOTFS_SQUASHFS_HASH) += bcm63xx_sec.o

ifneq ($(strip $(CONFIG_COMPAT)),)
    obj-y += compat_board.o
endif

ifneq ($(strip $(BUILD_IEEE1905)),)
obj-y += ieee1905_module.o    
endif

ifneq ($(strip $(CONFIG_MTD_NAND)),)
	EXTRA_CFLAGS += -DBUILD_NAND
endif

ifneq ($(strip $(CONFIG_SERIAL_BCM63XX_CONSOLE)),)
	EXTRA_CFLAGS += -DBRCM_SERIAL_CONSOLE
endif

ifneq ($(strip $(CONFIG_BCM_FTTDP)),)
    obj-y += xdsl_distpoint.o
endif

ifeq ($(BUILD_IEEE1905),y)
	EXTRA_CFLAGS += -DSUPPORT_IEEE1905
endif
 
EXTRA_CFLAGS += -DBRCM_BOARD_ID=\"$(BRCM_BOARD_ID)\" 
EXTRA_CFLAGS += -DBRCM_NUM_MAC_ADDRESSES=$(BRCM_NUM_MAC_ADDRESSES)
EXTRA_CFLAGS += -DBRCM_BASE_MAC_ADDRESS=\"$(BRCM_BASE_MAC_ADDRESS)\"
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/../flash 
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
	EXTRA_CFLAGS += -DWIRELESS
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

