# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2013 Freescale Semiconductor, Inc.

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y	+= t104xrdb.o
obj-y	+= cpld.o
obj-y	+= eth.o
obj-$(CONFIG_PCI)	+= pci.o
obj-$(CONFIG_FSL_DIU_FB)+= diu.o
endif
obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o
