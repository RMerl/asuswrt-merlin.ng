# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2010 Freescale Semiconductor, Inc.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
obj-y	+= spl_minimal.o
else
ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
endif
obj-y	+= p1022ds.o
obj-y	+= ddr.o
obj-$(CONFIG_FSL_DIU_FB) += diu.o
endif

obj-y	+= law.o
obj-y	+= tlb.o
