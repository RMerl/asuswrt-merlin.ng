# File: bcmdrivers/char/bcmprocfs/bcm963xx/Makefile
#
# Makefile for the BCM63xx bcmprocfs
#

obj-$(CONFIG_BCM_PROCFS) += bcmprocfs.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s

