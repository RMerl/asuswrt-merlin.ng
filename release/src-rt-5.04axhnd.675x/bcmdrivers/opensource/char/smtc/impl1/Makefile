#
# Makefile for the SMTC PMD driver
#

obj-$(CONFIG_BCM_SMTC) += smtc.o
smtc-objs += bcm_smtc.o A2_Table_80.o A2_Table_81.o A0_Lower.o A0_Upper.o A2_Lower.o

EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/char/plat-bcm/impl1/
EXTRA_CFLAGS +=  -Werror -Wall -I$(src)

-include $(TOPDIR)/Rules.make

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko *.mod.c modules.*
