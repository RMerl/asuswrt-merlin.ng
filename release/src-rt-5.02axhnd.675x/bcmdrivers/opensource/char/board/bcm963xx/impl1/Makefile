# Makefile for the bcm963xx board drivers
#


obj-y := board.o bcm63xx_flash.o bcm63xx_led.o bcm63xx_gpio.o pushbutton.o board_button.o \
	board_wl.o board_dg.o board_proc.o board_image.o board_util.o \
 	board_ioctl.o board_wd.o

obj-$(CONFIG_BCM_ROOTFS_SQUASHFS_HASH) += bcm63xx_sec.o

ifneq ($(strip $(CONFIG_COMPAT)),)
    obj-y += compat_board.o
endif

ifneq ($(strip $(BUILD_IEEE1905)),)
obj-y += ieee1905_module.o    
endif

ifneq ($(strip $(BUILD_6802_MOCA)),)
    obj-y += bbsi/bbsi.o
    obj-y += bbsi/bbsiExecSeq.o
    obj-y += moca/board_moca.o
    obj-y += moca/6802/init.o
else
    obj-y += spidevices.o
endif

ifneq ($(strip $(CONFIG_MTD_NAND)),)
	EXTRA_CFLAGS += -DBUILD_NAND
endif

ifneq ($(strip $(CONFIG_BCM_SERIAL_CONSOLE)),)
	EXTRA_CFLAGS += -DBRCM_SERIAL_CONSOLE
endif

ifneq ($(strip $(CONFIG_BCM_PLC_BOOT)),)
    obj-y += homeplugd_module.o
endif

ifneq ($(strip $(subst ", ,$(CONFIG_BCM_FTTDP))),)
    obj-y += dsldsp_operation.o
    obj-y += xdsl_distpoint.o
endif

ifeq ($(BUILD_IEEE1905),y)
	EXTRA_CFLAGS += -DSUPPORT_IEEE1905
endif
 
ifeq ($(strip $(BRCM_CHIP)),63268)
ifneq ($(strip $(BUILD_SECURE_BOOT)),)
ifeq ($(strip $(BUILD_NVRAM_3K)),y)
	EXTRA_CFLAGS += -DINC_BTRM_BOOT=1
endif
endif
endif
 
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/../flash 
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
	EXTRA_CFLAGS += -DWIRELESS
endif

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

