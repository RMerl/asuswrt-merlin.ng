# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2013-2015 Arcturus Networks, Inc.
# based on board/freescale/p1_p2_rdb_pc/Makefile
# original copyright follows:
# Copyright 2010-2011 Freescale Semiconductor, Inc.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL

obj-y	+= spl_minimal.o tlb.o law.o

else
ifdef CONFIG_SPL_BUILD
obj-y += spl.o
endif

obj-y        += ucp1020.o
obj-y        += ddr.o
obj-y        += law.o
obj-y        += tlb.o
obj-y        += cmd_arc.o

endif
