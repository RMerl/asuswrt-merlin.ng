# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_FPM_DRV) := fpm.o
fpm-objs += fpm_core.o fpm_dt.o fpm_proc.o
ifneq ($(CONFIG_BCM_FPM_TOKEN_HIST),)
fpm-objs  += fpm_hist.o
endif

#ifneq ($(CONFIG_BCM_CM),)
#ccflags-y += -Idrivers/bcm_media_gw/cm/include
#endif

#ifneq ($(CONFIG_BCM_FPM_TOKEN_HIST),)
#fpm-y += fpm_hist.o
#endif


EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean



##  erouter Makefile ##
#ccflags-y += -Werror -Wfatal-errors
#ccflags-y += -Idrivers/bcm_media_gw/include
#ifneq ($(CONFIG_BCM_CM),)
#ccflags-y += -Idrivers/bcm_media_gw/cm/include
#endif

#fpm-y += fpm_core.o fpm_dt.o fpm_proc.o
#ifneq ($(CONFIG_BCM_FPM_TOKEN_HIST),)
#fpm-y += fpm_hist.o
#endif
#obj-$(CONFIG_BCM_FPM) := fpm.o
