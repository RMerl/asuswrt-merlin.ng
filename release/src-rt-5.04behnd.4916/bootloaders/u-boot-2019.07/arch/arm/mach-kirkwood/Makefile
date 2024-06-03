# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2009
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>

obj-y	= cpu.o
obj-y	+= cache.o
obj-y	+= mpp.o

# cpu.o and cache.o contain CP15 instructions which cannot be run in
# Thumb state, so build them for ARM state even with CONFIG_SYS_THUMB_BUILD

CFLAGS_cpu.o := -marm
CFLAGS_cache.o := -marm
