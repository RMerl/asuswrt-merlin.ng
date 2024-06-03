# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_DQM_DRV) := dqm.o 
dqm-objs  += dqm_core.o dqm_dt.o dqm_dbg.o dqm_mon.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/fpm/impl$(CONFIG_BCM_FPM_DRV_IMPL)


clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean

##  erouter Makefile ##
#ccflags-y += -Werror -Wfatal-errors
#ccflags-y += -Idrivers/bcm_media_gw/include
#ccflags-y += -Idrivers/bcm_media_gw/fpm

#dqm-y += dqm_core.o dqm_dt.o dqm_dbg.o dqm_mon.o

#obj-$(CONFIG_BCM_DQM) := dqm.o
