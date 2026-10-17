# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2020 NXP
#

obj-y += lowlevel_init.o
obj-y += soc.o clock.o iomux.o pcc.o cgc.o rdc.o

ifeq ($(CONFIG_SPL_BUILD),y)
obj-y += upower/
endif
