# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_$(SPL_TPL_)USB_GADGET) += epautoconf.o config.o usbstring.o
obj-$(CONFIG_$(SPL_TPL_)USB_ETHER) += epautoconf.o config.o usbstring.o ether.o
obj-$(CONFIG_$(SPL_TPL_)USB_ETH_RNDIS) += rndis.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_USB_GADGET) += g_dnl.o
obj-$(CONFIG_SPL_DFU) += f_dfu.o
obj-$(CONFIG_SPL_USB_SDP_SUPPORT) += f_sdp.o
endif

# new USB gadget layer dependencies
ifdef CONFIG_USB_GADGET
obj-$(CONFIG_USB_GADGET_AT91) += at91_udc.o
obj-$(CONFIG_USB_GADGET_ATMEL_USBA) += atmel_usba_udc.o
obj-$(CONFIG_USB_GADGET_BCM_UDC_OTG_PHY) += bcm_udc_otg_phy.o
obj-$(CONFIG_USB_GADGET_DWC2_OTG) += dwc2_udc_otg.o
obj-$(CONFIG_USB_GADGET_DWC2_OTG_PHY) += dwc2_udc_otg_phy.o
obj-$(CONFIG_USB_GADGET_MAX3420) += max3420_udc.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_GADGET_DOWNLOAD) += g_dnl.o
obj-$(CONFIG_USB_FUNCTION_THOR) += f_thor.o
obj-$(CONFIG_DFU_OVER_USB) += f_dfu.o
obj-$(CONFIG_USB_FUNCTION_MASS_STORAGE) += f_mass_storage.o
obj-$(CONFIG_USB_FUNCTION_FASTBOOT) += f_fastboot.o
obj-$(CONFIG_USB_FUNCTION_SDP) += f_sdp.o
obj-$(CONFIG_USB_FUNCTION_ROCKUSB) += f_rockusb.o
obj-$(CONFIG_USB_FUNCTION_ACM)	+= f_acm.o
endif
endif

obj-$(CONFIG_CI_UDC) += ci_udc.o

# Devices not related to the new gadget layer depend on CONFIG_USB_DEVICE
# This is really only N900 and USBTTY now.
obj-$(CONFIG_USB_DEVICE) += core.o ep0.o
