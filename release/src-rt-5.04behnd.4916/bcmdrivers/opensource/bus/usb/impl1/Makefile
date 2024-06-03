MODULE_OBJS += bcm_bca_usb_ctrl.o bcm_bca_usb_hw_$(BRCM_CHIP).o bcm_bca_usb_utils.o 
MODULE_OBJS += bcm_bca_usb_xhci.o

EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD)

MODULE_NAME=bcm_bca_usb
$(MODULE_NAME)-objs += $(MODULE_OBJS)

obj-$(CONFIG_BCM_BCA_USB) += $(MODULE_NAME).o
clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
