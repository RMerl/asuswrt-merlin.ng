# SPDX-License-Identifier: GPL-2.0
#
# Copyright 2009-2010 Freescale Semiconductor, Inc.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL

obj-$(CONFIG_FSL_LAW) += law.o

else
obj-$(CONFIG_MPC85xx) += cpu.o
obj-$(CONFIG_MPC86xx) += cpu.o

obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-$(CONFIG_FSL_LBC) += fsl_lbc.o
obj-$(CONFIG_SYS_SRIO) += srio.o
obj-$(CONFIG_FSL_LAW) += law.o
obj-$(CONFIG_FSL_CORENET) += fsl_pamu.o pamu_table.o

endif
