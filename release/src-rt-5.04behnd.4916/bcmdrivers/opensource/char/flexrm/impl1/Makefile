# File: bcmdrivers/char/flexrm/bcm963xx/Makefile
#
# Makefile for the BCM63xx Crypto FlexRM module
#

obj-$(CONFIG_BCM_FLEXRM) += bcmflex.o

bcmflex-objs += flexrm.o

EXTRA_CFLAGS += -I$(src) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMBOARDPARMS_PATH)/$(BRCM_BOARD) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors

ifneq ($(strip $(CONFIG_BCM_SPU_HW_OFFLOAD)),)
EXTRA_CFLAGS += -I$(INC_RDPA_MW_PATH) $(INC_RDP_FLAGS)
endif
clean:
	rm -f core *.o *.a .*.cmd *.ko
