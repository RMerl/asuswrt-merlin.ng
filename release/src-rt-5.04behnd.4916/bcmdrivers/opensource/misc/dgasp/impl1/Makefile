# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_DGASP_DRV) := dgasp.o 

dgasp-objs += dgasp_drv.o

ifneq ($(filter y,$(CONFIG_BCM963158) $(CONFIG_BCM963178) $(CONFIG_BCM963146)),)
dgasp-objs += dgasp_pmc.o
else ifneq ($(filter y,$(CONFIG_BCM963138) $(CONFIG_BCM963148)),)
dgasp-objs += dgasp_afe.o
else
dgasp-objs += dgasp_periph.o
endif

ifneq ($(strip $(CONFIG_SERIAL_BCM63XX_CONSOLE)),)
	EXTRA_CFLAGS += -DBRCM_SERIAL_CONSOLE
endif

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g


-include $(TOPDIR)/Rules.make


clean:
	rm -f core *.ko *.o *.a *.s

