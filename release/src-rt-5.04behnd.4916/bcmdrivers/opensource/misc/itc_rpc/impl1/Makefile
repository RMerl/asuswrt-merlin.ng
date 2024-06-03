# Makefile for the bcm963xx board drivers
#

obj-$(CONFIG_BCM_ITC_RPC_DRV) := itc_rpc.o

itc_rpc-objs := itc_rpc_core.o itc_msg_q.o itc_rpc_dt.o itc_user.o itc_rpc_dbg.o


ifneq ($(CONFIG_BCM_ITC_RPC_DQM),)
itc_rpc-objs += itc_rpc_fifo_dqm.o
endif

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/fpm/impl$(CONFIG_BCM_FPM_DRV_IMPL)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/dqm/impl$(CONFIG_BCM_DQM_DRV_IMPL)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean



##  erouter Makefile ##
#ccflags-y := -Werror -Wfatal-errors
#ccflags-y += -Idrivers/bcm_media_gw/include
#ccflags-y += -Idrivers/bcm_media_gw/dqm
#ccflags-y += -Idrivers/bcm_media_gw/fpm

#itc_rpc-y := itc_rpc_core.o itc_msg_q.o itc_rpc_dt.o itc_user.o itc_rpc_dbg.o

#ifneq ($(CONFIG_BCM_ITC_RPC_DQM),)
#itc_rpc-y += itc_rpc_fifo_dqm.o
#endif
#itc_rpc-y += rpc_power_test.o
#itc_rpc-y += rpc_led.o

#obj-$(CONFIG_BCM_ITC_RPC) := itc_rpc.o
