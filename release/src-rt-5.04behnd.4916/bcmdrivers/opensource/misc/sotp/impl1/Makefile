# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_SOTP) := sotp.o 

ifneq ($(filter y,$(CONFIG_BCM947622) $(CONFIG_BCM963158) $(CONFIG_BCM963178) $(CONFIG_BCM96756) $(CONFIG_BCM96856) $(CONFIG_BCM96858)),)
# 28nm Legacy SOTP
sotp-objs += sotp_drv.o
sotp-objs += sotp_base_funcs.o
else
# 16nm SKP based SOTP
sotp-objs += sotp_drv_skp.o
sotp-objs += skp_base_funcs.o
endif

EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -g


-include $(TOPDIR)/Rules.make


clean:
	rm -f core *.ko *.o *.a *.s

