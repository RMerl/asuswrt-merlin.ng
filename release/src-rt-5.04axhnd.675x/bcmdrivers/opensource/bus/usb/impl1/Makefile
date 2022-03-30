
bcm_bca_usb-objs := bcm_bca_usb_ctrl.o bcm_bca_usb_hw_$(BRCM_CHIP).o bcm_bca_usb_utils.o
bcm_bca_usb-objs += bcm_bca_usb_xhci.o bcm_bca_usb_ehci.o bcm_bca_usb_ohci.o

obj-$(CONFIG_BCM_BCA_USB) += bcm_bca_usb.o

#obj-$(CONFIG_BCM_BCA_USB) += bcm_bca_usb_ctrl.o bcm_bca_usb_hw_$(BRCM_CHIP).o bcm_bca_usb_utils.o
#obj-$(CONFIG_BCM_BCA_USB) += bcm_bca_usb_xhci.o bcm_bca_usb_ehci.o bcm_bca_usb_ohci.o
EXTRA_CFLAGS += -Werror -Wfatal-errors
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)
EXTRA_CFLAGS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)/pmc
EXTRA_CFLAGS += -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

clean:
	rm -f core *.o *.a *.s .*.cmd *.ko
.PHONY: clean
