# File: bcmdrivers/opensource/char/timer
#
# Makefile for the BCM963xx external timer driver

obj-$(CONFIG_BCM_EXT_TIMER) += bcm963xx_timer.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -Werror -Wfatal-errors

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

