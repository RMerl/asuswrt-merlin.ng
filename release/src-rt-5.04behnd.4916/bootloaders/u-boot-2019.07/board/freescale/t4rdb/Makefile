#
# Copyright 2014 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier: GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
else
obj-$(CONFIG_TARGET_T4160RDB)	+= t4240rdb.o
obj-$(CONFIG_TARGET_T4240RDB)	+= t4240rdb.o
obj-y			+= cpld.o
obj-y			+= eth.o
obj-$(CONFIG_PCI)	+= pci.o
endif

obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o
