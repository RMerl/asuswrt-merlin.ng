obj-$(CONFIG_BCM_BCA_GPIO) += gpio-bcm-bca.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
