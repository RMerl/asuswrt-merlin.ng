# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-y	= cpu.o

ifneq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_EFI_LOADER) += sctlr.o
endif
