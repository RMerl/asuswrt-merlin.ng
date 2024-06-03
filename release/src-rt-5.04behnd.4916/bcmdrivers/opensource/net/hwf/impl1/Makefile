# File: bcmdrivers/broadcom/net/hwf/impl1/Makefile
#
# Makefile for the HW Firewall module
#
bcm_hwf-objs := hwf.o
obj-$(CONFIG_BCM_HW_FIREWALL) += bcm_hwf.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(INC_BCMLIBS_PATH)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) $(EXTRA_CFLAGS_RDPA) $(INC_RDP_FLAGS)
EXTRA_CFLAGS += -Werror -Wfatal-errors 

clean:
	rm -f core *.ko *.o *.a *.s

