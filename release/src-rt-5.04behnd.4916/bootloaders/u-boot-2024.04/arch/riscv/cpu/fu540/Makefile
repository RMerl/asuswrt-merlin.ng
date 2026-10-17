# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2020 SiFive, Inc
# Pragnesh Patel <pragnesh.patel@sifive.com>

ifeq ($(CONFIG_SPL_BUILD),y)
obj-y += spl.o
else
obj-y += dram.o
obj-y += cpu.o
endif
