# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

BRCM_CHIP := $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))

ifdef CONFIG_BCMBCA_XRDP_ETH

ifdef CONFIG_BCMBCA_PMC
EXTRA_CFLAGS += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/pmc
endif

ifdef CONFIG_BCMBCA_XRDP
EXTRA_CFLAGS += -I$(srctree)/drivers/net/bcmbca/xrdp
endif

EXTRA_CFLAGS += -Werror
obj-$(CONFIG_BCMBCA_XRDP_ETH) += bcmbca_xrdp_eth.o
obj-$(CONFIG_BCMBCA_LPORT) += lport/

ifdef CONFIG_BCMBCA_XRDP_PON
EXTRA_CFLAGS += -DCONFIG_BCM_PON -DCONFIG_BCM_PON_XRDP -DBCM_PON_XRDP
endif

ifdef CONFIG_BCMBCA_XRDP_DSL
EXTRA_CFLAGS += -DCONFIG_BCM_DSL -DCONFIG_BCM_DSL_XRDP -DBCM_DSL_XRDP
endif

CONFIG_XRDP_FULL ?= 0

ifeq ($(strip $(CONFIG_XRDP_FULL)),1)
obj-$(CONFIG_BCMBCA_XRDP) += xrdp_full/
# Execute prepare links first
$(obj)/xrdp_full: prepare_links
prepare_links:
	$(MAKE) -C $(srctree)/drivers/net/bcmbca/xrdp_full $@ BRCM_CHIP=$(BRCM_CHIP)
else
obj-$(CONFIG_BCMBCA_XRDP) += xrdp/
endif

endif


ifdef CONFIG_BCMBCA_RDP_ETH

KBUILD_CPPFLAGS += -DLEGACY_RDP -DRDD_BASIC -DBCM_DSL_RDP

ifdef CONFIG_BCM63138
KBUILD_CPPFLAGS += -DDSL_63138
endif

ifdef CONFIG_BCM63148
KBUILD_CPPFLAGS += -DDSL_63148
endif

UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/rdp
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/$(subst \",,$(CONFIG_SYS_SOC))/rdp_fw
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/pmc

obj-y += bcmbca_rdp_eth.o

endif

ifdef CONFIG_BCMBCA_IKOS
ifdef CONFIG_BCMBCA_XRDP_PON
obj-y += bcmbca_ikos_network.o
endif
endif

ifdef CONFIG_BCMBCA_SWITCH
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/pmc
ifdef_any_of = $(filter-out undefined,$(foreach v,$(1),$(origin $(v))))
obj-y += bcm_ethsw_impl1.o bcm_ethsw_phy.o
# the following 3 SOCs have specific PHY configurations
ifneq ($(call ifdef_any_of,CONFIG_BCM63138 CONFIG_BCM63148 CONFIG_BCM63158),)
obj-y += bcm_ethsw_phy_$(subst bcm,,$(subst ",,$(CONFIG_SYS_SOC))).o
endif
# the following 4 old SOCs don't use dt phy init so need manual led init
ifneq ($(call ifdef_any_of,CONFIG_BCM63138 CONFIG_BCM63148 CONFIG_BCM4908 CONFIG_BCM63158),)
ifdef CONFIG_BCMBCA_PHY
EXTRA_CFLAGS += -DMANUAL_LED_INIT
endif
endif
endif

obj-$(CONFIG_BCMBCA_SYSPORT_V1) += bcmbca_sysport_v1.o
ifndef CONFIG_BCMBCA_IKOS
obj-$(CONFIG_BCMBCA_SYSPORT_V2) += bcmbca_sysport_v2.o
endif
obj-$(CONFIG_BCMBCA_SF2_GMAC) += bcm-sf2-eth-gmac.o

ifdef CONFIG_BCMBCA_EXT_SWITCH
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/pmc
obj-y += bcm_ethsw_ext.o bcm_ethsw_phy.o
endif

ifneq ($(findstring _$(strip $(BRCM_CHIP))_,_4908_63178_47622_6756_63146_4912_6813_6765_),)
EXTRA_CFLAGS += -DDSL_DEVICES
endif

ifeq ($(BUILD_NAME), $(filter $(BUILD_NAME), RT-AX58U_V2 RT-AX3000N BR63 RT-BE58U TUF-BE3600))
obj-y += rtk/
endif

ifeq ($(or $(GTBE98),$(GTBE98_PRO),$(GTBE96),$(GTBE19000),$(RTBE92U),$(RTBE95U)),y)
obj-y += rtl8372/
endif

ifdef CONFIG_BCMBCA_PHY
EXTRA_CFLAGS += -I$(srctree)/drivers/net/bcmbca/phy
endif
obj-$(CONFIG_BCMBCA_PHY) += phy/
