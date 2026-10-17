# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

extra-y	= start.o
obj-y	= irq.o
obj-y	+= interrupts.o cache.o exception.o cpuinfo.o relocate.o
obj-$(CONFIG_XILINX_MICROBLAZE0_PVR) += pvr.o
obj-$(CONFIG_SPL_BUILD)	+= spl.o
