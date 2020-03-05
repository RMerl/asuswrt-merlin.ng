#
# Makefile for the optical wan detection
#

obj-$(CONFIG_BCM_OPTICALDET) += opticaldet.o
opticaldet-objs += detect_dev.o
opticaldet-objs += detect_dev_trx_data.o
opticaldet-objs += trx_activation.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

EXTRA_CFLAGS +=  -Werror -Wall

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
