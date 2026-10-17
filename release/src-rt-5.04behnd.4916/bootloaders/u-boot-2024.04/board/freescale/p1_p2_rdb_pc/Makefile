# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2010-2011 Freescale Semiconductor, Inc.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifndef CONFIG_TPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif
endif

ifdef MINIMAL
obj-y	+= spl_minimal.o
else
ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
endif
obj-y	+= p1_p2_rdb_pc.o
obj-y	+= ddr.o
endif

obj-y	+= law.o
obj-y	+= tlb.o
