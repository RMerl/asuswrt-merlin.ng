# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2005-2007 Samsung Electronics.
# Kyungmin Park <kyungmin.park@samsung.com>

ifndef	CONFIG_SPL_BUILD
obj-$(CONFIG_CMD_ONENAND)	:= onenand_uboot.o onenand_base.o onenand_bbt.o
obj-$(CONFIG_SAMSUNG_ONENAND)	+= samsung.o
else
obj-y				:= onenand_spl.o
endif
