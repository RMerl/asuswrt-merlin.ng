# SPDX-License-Identifier: GPL-2.0+
#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs

obj-$(CONFIG_USB_MUSB_GADGET) += musb_gadget.o musb_gadget_ep0.o musb_core.o
obj-$(CONFIG_USB_MUSB_GADGET) += musb_uboot.o
obj-$(CONFIG_USB_MUSB_HOST) += musb_host.o musb_core.o musb_uboot.o
obj-$(CONFIG_USB_MUSB_DSPS) += musb_dsps.o
obj-$(CONFIG_USB_MUSB_DA8XX)	+= da8xx.o
obj-$(CONFIG_USB_MUSB_AM35X) += am35x.o
obj-$(CONFIG_USB_MUSB_MT85XX) += mt85xx.o
obj-$(CONFIG_USB_MUSB_OMAP2PLUS) += omap2430.o
obj-$(CONFIG_USB_MUSB_PIC32) += pic32.o
obj-$(CONFIG_USB_MUSB_SUNXI) += sunxi.o
obj-$(CONFIG_USB_MUSB_TI) += ti-musb.o
obj-$(CONFIG_USB_MUSB_UX500) += ux500.o

ccflags-y := $(call cc-option,-Wno-unused-variable) \
		$(call cc-option,-Wno-unused-but-set-variable) \
		$(call cc-option,-Wno-unused-label)
