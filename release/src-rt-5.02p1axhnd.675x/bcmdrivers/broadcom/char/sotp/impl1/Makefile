# Makefile for the bcm963xx board drivers
#


obj-$(CONFIG_BCM_SOTP) := sotp.o 
sotp-objs += sotp_drv.o 
sotp-objs += ../../../../../shared/broadcom/drivers/sotp/sotp_base_funcs.o 

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g


-include $(TOPDIR)/Rules.make


clean:
	rm -f core *.ko *.o *.a *.s

