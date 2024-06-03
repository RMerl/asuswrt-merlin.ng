obj-$(CONFIG_BCM_UBUS) += bcm_ubus.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

.PHONY: clean
