# Makefile for the bcm963xx board drivers
#


ifneq ($(strip $(BRCM_IKOS)),)
obj-y := board_ikos.o board_dt.o shared_utils.o
obj-$(CONFIG_BRCM_QEMU) += board_misc_hw_init.o
else
obj-y := board.o bcm63xx_flash.o board_flash_api.o board_flash_common.o \
	board_wl.o board_proc.o board_image.o board_util.o board_dt.o\
	board_ioctl.o shared_utils.o board_nvram.o board_misc_hw_init.o

obj-$(CONFIG_BCM_ROOTFS_SQUASHFS_HASH) += bcm63xx_sec.o

ifneq ($(strip $(CONFIG_MTD_NAND)),)
	EXTRA_CFLAGS += -DBUILD_NAND
endif

ifneq ($(strip $(CONFIG_SERIAL_BCM63XX_CONSOLE)),)
	EXTRA_CFLAGS += -DBRCM_SERIAL_CONSOLE
endif

ifneq ($(strip $(CONFIG_BCM_FTTDP)),)
    obj-y += xdsl_distpoint.o
endif

endif

ifneq ($(strip $(CONFIG_COMPAT)),)
    obj-y += compat_board.o
endif

 ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_68880_6837_),) # [DM XXX]stub for OTP and PMC API;s
	obj-y += temp_board_smc_api.o
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

