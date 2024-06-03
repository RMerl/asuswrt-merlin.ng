#
# Makefile for voice board params driver
#

obj-$(CONFIG_BCM_VOICE_PARAMS)	+= boardparms_voice.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -DEXPORT_SYMTAB

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.ko *.o *.a *.s