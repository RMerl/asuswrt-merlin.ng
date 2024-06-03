# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2013 Freescale Semiconductor, Inc.

obj-y	+= generic.o
obj-y	+= timer.o

MKIMAGEFLAGS_u-boot.vyb = -T vybridimage

u-boot.vyb: u-boot.imx
	$(call if_changed,mkimage)
