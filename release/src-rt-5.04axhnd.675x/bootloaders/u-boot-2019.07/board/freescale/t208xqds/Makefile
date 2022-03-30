#
# Copyright 2013 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:      GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-$(CONFIG_TARGET_T2080QDS) += t208xqds.o eth_t208xqds.o
obj-$(CONFIG_TARGET_T2081QDS) += t208xqds.o eth_t208xqds.o
obj-$(CONFIG_PCI)      += pci.o
endif

obj-y   += ddr.o
obj-y   += law.o
obj-y   += tlb.o
