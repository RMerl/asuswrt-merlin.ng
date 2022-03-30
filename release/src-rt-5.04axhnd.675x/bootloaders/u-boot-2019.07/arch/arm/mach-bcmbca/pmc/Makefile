# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

# Temperarily put the pmc impl in arch folder. Need to 
# move to driver and could try to use the power-domain 
# uclass but it is very simple form now and need to add
# the interface we need

EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/ubus

obj-y +=			\
	pmc_drv.o		\
	pmc_drv_bootloader.o	\
	clk_rst.o		\

obj-$(CONFIG_BCMBCA_PMC_SWITCH) += pmc_switch.o
obj-$(CONFIG_BCMBCA_PMC_RDP) += pmc_rdp.o
obj-$(CONFIG_BCMBCA_PMC_XRDP) += pmc_xrdp.o
obj-$(CONFIG_BCMBCA_PMC_LPORT) += pmc_lport.o
obj-$(CONFIG_BCMBCA_PMC_SYSPORT) += pmc_sysport.o
obj-$(CONFIG_BCMBCA_PMC_USB) += pmc_usb.o
