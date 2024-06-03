# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_SOTP) := sotp.o 

ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_4912_),)
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_63146_),)
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6813_),)
ifeq ($(findstring _$(strip $(BRCM_CHIP))_,_6765_),)
BUILD_SOTP_SKP := n
endif
endif
endif
endif

ifeq ($(strip $(BUILD_SOTP_SKP)),n)
sotp-objs += sotp_drv.o
sotp-objs += sotp_base_funcs.o 
else
sotp-objs += sotp_drv_skp.o
sotp-objs += skp_base_funcs.o 
endif



EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g


-include $(TOPDIR)/Rules.make


clean:
	rm -f core *.ko *.o *.a *.s

