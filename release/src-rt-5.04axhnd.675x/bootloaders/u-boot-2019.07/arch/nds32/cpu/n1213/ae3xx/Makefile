# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2009
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>
#
# Copyright (C) 2011 Andes Technology Corporation
# Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
# Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>

obj-y	:= cpu.o timer.o
obj-y	+= lowlevel_init.o

ifndef CONFIG_SKIP_TRUNOFF_WATCHDOG
obj-y	+= watchdog.o
endif
