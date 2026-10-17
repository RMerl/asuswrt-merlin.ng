# File: bcmdrivers/opensource/net/bnl/impl1/Makefile
#
# Makefile for the BNL module
#
bcm_bnl-objs := bnl_main.o bnl_ioctl.o
obj-$(CONFIG_BCM_BNL) += bcm_bnl.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(INC_BCMLIBS_PATH)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) $(EXTRA_CFLAGS_RDPA) $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -Werror -Wfatal-errors 

clean:
	rm -f core *.ko *.o *.a *.s

