# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2013 Freescale Semiconductor, Inc.
#

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
obj-y	+= c29xpcie.o
obj-y	+= cpld.o
obj-y	+= ddr.o
endif

obj-y	+= law.o
obj-y	+= tlb.o
