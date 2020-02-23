# 3.2+ Kernel
ifneq ($(strip $(BRCM_CHIP)),47189)
obj-y +=               \
	bcmHsSpi.o        
endif

obj-y +=               \
    bcmLegSpi.o 

obj-y +=               \
    bcmSpiRes.o 

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/pmc

