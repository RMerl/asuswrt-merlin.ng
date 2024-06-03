obj-$(CONFIG_BCM_BCA_LED) += bcm_bca_led_ctrl.o bcm_bca_cled_ctrl.o bcm_bca_leds.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(BRCMDRIVERS_DIR)/opensource/phy

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
