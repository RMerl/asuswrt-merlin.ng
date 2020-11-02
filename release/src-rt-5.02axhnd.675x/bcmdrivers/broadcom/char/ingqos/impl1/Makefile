# File: bcmdrivers/char/ingqos/bcm963xx/Makefile
#
# Makefile for the BCM63xx ingqos module
#

bcm_ingqos-objs   := ingqos.o
obj-$(CONFIG_BCM_INGQOS) += bcm_ingqos.o

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) $(EXTRA_CFLAGS_RDPA) $(INC_RDP_FLAGS)

EXTRA_CFLAGS += -Werror -Wfatal-errors 

clean:
	rm -f core *.ko *.o *.a *.s



