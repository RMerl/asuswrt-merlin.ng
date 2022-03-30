obj-$(CONFIG_BCM_SPIDEV_REV2) += bcmspidev_rev2.o

EXTRA_CFLAGS += -Werror -Wall
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -DEXPORT_SYMTAB
EXTRA_CFLAGS += -g

-include $(TOPDIR)/Rules.make

clean:
	rm -r core *.o *.a *.s .*.cmd *.ko

