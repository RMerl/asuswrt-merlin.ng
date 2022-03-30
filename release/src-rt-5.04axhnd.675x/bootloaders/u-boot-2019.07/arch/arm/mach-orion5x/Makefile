# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
#
# Based on original Kirkwood support which is
# (C) Copyright 2009
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>

obj-y	= cpu.o
obj-y	+= dram.o
obj-y	+= timer.o

ifndef CONFIG_SKIP_LOWLEVEL_INIT
obj-y	+= lowlevel_init.o
endif

# some files can only build in ARM or THUMB2, not THUMB1

ifdef CONFIG_$(SPL_)SYS_THUMB_BUILD
ifndef CONFIG_HAS_THUMB2

CFLAGS_cpu.o := -marm

endif
endif
