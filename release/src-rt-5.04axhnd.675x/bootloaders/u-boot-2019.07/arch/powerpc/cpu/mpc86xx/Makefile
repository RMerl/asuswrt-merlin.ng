# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2007 Freescale Semiconductor, Inc.
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com
#
# (C) Copyright 2004 Freescale Semiconductor. (MC86xx Port)
# Jeff Brown
#

extra-y	= start.o
extra-y	+= traps.o

obj-y += cache.o
obj-$(CONFIG_MP) += release.o

obj-y	+= cpu.o
obj-y	+= cpu_init.o
obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-y	+= interrupts.o
obj-$(CONFIG_MP) += mp.o
obj-$(CONFIG_ARCH_MPC8610) += mpc8610_serdes.o
obj-$(CONFIG_ARCH_MPC8641) += mpc8641_serdes.o
obj-y	+= speed.o
