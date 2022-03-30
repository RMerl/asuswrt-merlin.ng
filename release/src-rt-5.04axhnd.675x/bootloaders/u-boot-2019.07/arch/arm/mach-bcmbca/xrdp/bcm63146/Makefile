# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/xrdp -I$(srctree)/arch/$(ARCH)/mach-bcmbca/xrdp/$(subst \",,$(CONFIG_SYS_SOC))
ifndef CONFIG_BCMBCA_XRDP_GPL
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/xrdp/xrdp_full
endif

obj-y += rdd_data_structures_auto.o rdd_map_auto.o
obj-y += xrdp_drv_drivers_common_ag.o
obj-y += XRDP_AG.o
ifdef CONFIG_BCMBCA_XRDP_GPL
# handcrafted version is to take out the definition of unused registers
# in GPL version.  Those waste a lot of rodata memory.
obj-y += xrdp_drv_psram.o xrdp_drv_sbpm.o xrdp_drv_unimac_rdp.o
obj-y += xrdp_drv_rnr_regs.o
obj-y += XRDP_PSRAM_GPL.o XRDP_SBPM_GPL.o
obj-y += XRDP_UNIMAC_RDP_GPL.o XRDP_RNR_REGS_GPL.o
endif

