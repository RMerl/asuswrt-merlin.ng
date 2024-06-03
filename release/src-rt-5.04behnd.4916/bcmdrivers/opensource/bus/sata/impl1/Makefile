obj-$(CONFIG_BCM_BCA_SATA) += bcm_bca_sata.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/ata

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
