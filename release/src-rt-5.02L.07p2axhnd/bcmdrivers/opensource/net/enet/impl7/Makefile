
MODULE_NAME=bcm_enet

$(MODULE_NAME)-objs += $(MODULE_NAME)$(PRBM_EXT).o

obj-$(CONFIG_BCM_ENET) += $(MODULE_NAME).o

clean:
	rm -f core *.o *.a *.s *.ko .*.cmd .*.o.d Module.symvers modules.builtin modules.order *.mod.c
	rm -rf .tmp_versions

