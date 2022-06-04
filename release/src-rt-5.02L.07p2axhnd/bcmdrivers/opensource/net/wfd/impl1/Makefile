
MODULE_NAME=wfd

$(MODULE_NAME)-objs += $(MODULE_NAME)$(PRBM_EXT).o

obj-$(CONFIG_BCM_WIFI_FORWARDING_DRV) += $(MODULE_NAME).o

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko

