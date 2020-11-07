# File: bcmdrivers/char/profiler/bcm963xx/Makefile
#
# Makefile for the BCM63xx Profiling API driver.
#

obj-$(CONFIG_BCM_BCMPROF) += profdrvdd.o

profdrvdd-objs := profdrv.o profiler.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -fno-exceptions

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s
