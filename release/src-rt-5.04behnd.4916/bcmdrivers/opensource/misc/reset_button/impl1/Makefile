obj-$(CONFIG_BCM_RESET_BUTTON) += reset_button.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)

ifneq ($(strip $(CONFIG_BCM_WLAN)),)
    EXTRA_CFLAGS += -DWIRELESS
endif
 
clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
