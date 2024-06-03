# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

# Temperarily put the pmc impl in arch folder. Need to 
# move to driver and could try to use the power-domain 
# uclass but it is very simple form now and need to add
# the interface we need

EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/ubus
debug_cmd := $(patsubst %.c,%.o,$(wildcard $(srctree)/arch/$(ARCH)/mach-bcmbca/pmc/*setvdd.c))
debug_cmd := $(notdir $(debug_cmd))

ifeq ($(CONFIG_SMC_BASED)b$(CONFIG_BCMBCA_NO_SMC_BOOT)c,ybc)
    obj-y +=            \
    smc_clk_rst.o       \
    smc_drv_dt.o        \
    clk_rpc_svc.o       \
    power_rpc_svc.o     \
    smc_log.o           \
    $(debug_cmd)        \

obj-$(CONFIG_BCMBCA_PMC_XRDP) += smc_xrdp.o
obj-$(CONFIG_BCMBCA_PMC_XPORT) += smc_xport.o
obj-$(CONFIG_BCMBCA_PMC_USB) += smc_usb.o
obj-$(CONFIG_BCMBCA_PMC_ETHTOP) += smc_ethtop.o
obj-$(CONFIG_BCMBCA_PMC_ETHTOP) += smc_rccore.o
else
obj-y +=                    \
    pmc_drv.o               \
    pmc_drv_bootloader.o    \
    clk_rst.o               \
    pmc_drv_dt.o            \
 
obj-$(CONFIG_BCMBCA_PMC_SWITCH) += pmc_switch.o
obj-$(CONFIG_BCMBCA_PMC_RDP) += pmc_rdp.o
obj-$(CONFIG_BCMBCA_PMC_XRDP) += pmc_xrdp.o
obj-$(CONFIG_BCMBCA_PMC_XPORT) += pmc_xport.o
obj-$(CONFIG_BCMBCA_PMC_LPORT) += pmc_lport.o
obj-$(CONFIG_BCMBCA_PMC_SYSPORT) += pmc_sysport.o
obj-$(CONFIG_BCMBCA_PMC_USB) += pmc_usb.o
obj-$(CONFIG_BCMBCA_PMC_A9_CORE) += pmc_a9_core.o
endif
