# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2014 - 2015 Xilinx, Inc.
# Michal Simek <michal.simek@xilinx.com>

obj-y	+= clk.o
obj-y	+= cpu.o
obj-$(CONFIG_MP)	+= mp.o
obj-$(CONFIG_SPL_BUILD) += spl.o handoff.o
obj-$(CONFIG_ZYNQMP_PSU_INIT_ENABLED)	+= psu_spl_init.o
