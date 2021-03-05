# File: bcmdrivers/char/spudd/bcm963xx/Makefile
#
# Makefile for the BCM63xx Crypto PDC module
#

obj-$(CONFIG_BCM_PDC) += bcmpdc.o

ifneq ($(PREBUILT_EXTRAMOD),1)
bcmpdc-objs += pdc.o pdc_debug.o
else
bcmpdc-objs += bcmpdc_preb.o
endif

EXTRA_CFLAGS += -I$(src) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

clean:
	rm -f core *.o *.a .*.cmd *.ko
