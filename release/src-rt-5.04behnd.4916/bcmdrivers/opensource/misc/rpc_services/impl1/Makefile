# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_RPC_SERVICES_DRV) := rpc_services.o
rpc_services-y := misc_services.o misc_services_dev.o pwm_service.o power_services.o


EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/fpm/impl$(CONFIG_BCM_FPM_DRV_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/itc_rpc/impl$(CONFIG_ITC_RPC_DRV_IMPL)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean





##  erouter Makefile ##
#ccflags-y := -Werror -Wfatal-errors
#ccflags-y += -Idrivers/bcm_media_gw/include
#ccflags-y += -Idrivers/bcm_media_gw/itc_rpc
#ccflags-y += -Idrivers/bcm_media_gw/fpm
#ccflags-y += -Idrivers/bcm_media_gw/rpc_services
#ccflags-y += -Idrivers/bcm_media_gw/dqnet

#rpc_services-y := misc_services.o misc_services_dev.o pwm_service.o power_services.o
#obj-$(CONFIG_BCM_RPC_SERVICES) := rpc_services.o
