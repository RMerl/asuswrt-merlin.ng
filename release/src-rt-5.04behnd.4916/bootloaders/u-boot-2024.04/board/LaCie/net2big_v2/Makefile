# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
#
# Based on Kirkwood support:
# (C) Copyright 2009
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>

obj-y	:= net2big_v2.o ../common/common.o
ifneq ($(and $(CONFIG_KIRKWOOD_GPIO),$(CONFIG_NET2BIG_V2)),)
obj-y	+= ../common/cpld-gpio-bus.o
endif
