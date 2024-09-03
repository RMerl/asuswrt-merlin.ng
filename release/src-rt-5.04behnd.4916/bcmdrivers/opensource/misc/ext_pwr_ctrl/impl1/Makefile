obj-$(CONFIG_BCM_EXT_PWR_CTRL_DRV) += ext_pwr_ctrl_drv.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I. -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)  -I$(INC_BRCMSHARED_PRIV_PATH)/$(BRCM_BOARD) 
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/itc_rpc/impl$(CONFIG_ITC_RPC_DRV_IMPL)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean

