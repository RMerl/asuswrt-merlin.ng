# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2019 Broadcom Ltd
#

CHIP_ID := $(patsubst "bcm%",%,$(CONFIG_SYS_SOC))

# Must defined
KBUILD_CPPFLAGS += -DBDMF_NO_TRACE -D_BYTE_ORDER_LITTLE_ENDIAN_
KBUILD_CPPFLAGS += -DFIRMWARE_LITTLE_ENDIAN -DXRDP_SIMPLE_NET

UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/xrdp
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/xrdp/$(subst \",,$(CONFIG_SYS_SOC))

obj-y += rdp_platform.o rdd_common.o rdd_cpu_tx_ring.o rdd_cpu_rx.o
obj-y += rdp_cpu_ring.o access_logging.o ru.o

ifdef CONFIG_BCMBCA_XRDP_GPL
obj-y += data_path_init_basic.o  rdpa_gpl_sbpm.o
else
KBUILD_CPPFLAGS += -DCONFIG_GPL_RDP_GEN
UBOOTINCLUDE += -I$(srctree)/arch/$(ARCH)/mach-bcmbca/xrdp/xrdp_full
obj-y += xrdp_full/
endif

obj-$(CONFIG_BCM6846) += bcm6846/
obj-$(CONFIG_BCM6856) += bcm6856/
obj-$(CONFIG_BCM6858) += bcm6858/
obj-$(CONFIG_BCM6878) += bcm6878/
obj-$(CONFIG_BCM63146) += bcm63146/

