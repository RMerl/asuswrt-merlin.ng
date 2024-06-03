obj-$(CONFIG_BCM_UBUS) += bcm_ubus.o bcm_ubus_dt.o $(BRCM_CHIP)/bcm_ubus_tbl.o
obj-$(CONFIG_BCM_UBUS4_DCM) += bcm_ubus_dcm.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/misc/ubus/impl$(CONFIG_BCM_UBUS_IMPL)/$(BRCM_CHIP)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

.PHONY: clean
