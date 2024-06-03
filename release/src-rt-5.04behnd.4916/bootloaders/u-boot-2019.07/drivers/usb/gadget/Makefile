# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_USB_GADGET) += epautoconf.o config.o usbstring.o
obj-$(CONFIG_USB_ETHER) += epautoconf.o config.o usbstring.o

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
obj-$(CONFIG_USB_GADGET_FOTG210) += fotg210.o
obj-$(CONFIG_CI_UDC)	+= ci_udc.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_USB_GADGET_DOWNLOAD) += g_dnl.o
obj-$(CONFIG_USB_FUNCTION_THOR) += f_thor.o
obj-$(CONFIG_DFU_OVER_USB) += f_dfu.o
obj-$(CONFIG_USB_FUNCTION_MASS_STORAGE) += f_mass_storage.o
obj-$(CONFIG_USB_FUNCTION_FASTBOOT) += f_fastboot.o
obj-$(CONFIG_USB_FUNCTION_SDP) += f_sdp.o
obj-$(CONFIG_USB_FUNCTION_ROCKUSB) += f_rockusb.o
endif
endif
ifdef CONFIG_USB_ETHER
obj-y += ether.o
obj-$(CONFIG_USB_ETH_RNDIS) += rndis.o
obj-$(CONFIG_CI_UDC)	+= ci_udc.o
obj-$(CONFIG_CPU_PXA25X) += pxa25x_udc.o
else
# Devices not related to the new gadget layer depend on CONFIG_USB_DEVICE
ifdef CONFIG_USB_DEVICE
obj-y += core.o
obj-y += ep0.o
obj-$(CONFIG_DW_UDC) += designware_udc.o
obj-$(CONFIG_CPU_PXA27X) += pxa27x_udc.o
endif
endif
