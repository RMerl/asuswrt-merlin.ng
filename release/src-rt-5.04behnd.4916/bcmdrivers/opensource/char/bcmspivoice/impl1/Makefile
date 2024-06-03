obj-$(CONFIG_BCM_SPIDEV_VOICE) += bcmspidev_voice.o

EXTRA_CFLAGS += -Werror -Wall
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -DEXPORT_SYMTAB

-include $(TOPDIR)/Rules.make

clean:
	rm -r core *.o *.a *.s .*.cmd *.ko
