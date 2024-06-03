# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2012 Freescale Semiconductor, Inc.

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
else
obj-y	+= b4860qds.o
obj-$(CONFIG_TARGET_B4860QDS)	+= eth_b4860qds.o
obj-$(CONFIG_TARGET_B4420QDS)	+= eth_b4860qds.o
obj-$(CONFIG_PCI)	+= pci.o
endif

obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o
