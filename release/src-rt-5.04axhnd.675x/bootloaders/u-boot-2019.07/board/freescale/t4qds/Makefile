# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2012 Freescale Semiconductor, Inc.

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
else
obj-$(CONFIG_TARGET_T4160QDS)	+= t4240qds.o eth.o
obj-$(CONFIG_TARGET_T4240QDS)	+= t4240qds.o eth.o
obj-$(CONFIG_PCI)	+= pci.o
endif

obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o
