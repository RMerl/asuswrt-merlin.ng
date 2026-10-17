#
# Copyright 2014 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:      GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
else
obj-$(CONFIG_TARGET_T2080RDB) += t208xrdb.o eth_t208xrdb.o cpld.o
endif

obj-y   += ddr.o
obj-y   += law.o
obj-y   += tlb.o
