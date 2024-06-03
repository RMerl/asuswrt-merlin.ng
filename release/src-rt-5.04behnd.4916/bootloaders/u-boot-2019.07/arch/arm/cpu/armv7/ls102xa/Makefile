#
# Copyright 2014 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y	+= cpu.o
obj-y	+= clock.o
obj-y	+= timer.o
obj-y	+= fsl_epu.o
obj-y	+= soc.o

obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-$(CONFIG_SYS_HAS_SERDES) += fsl_ls1_serdes.o ls102xa_serdes.o
obj-$(CONFIG_SPL) += spl.o

ifdef CONFIG_ARMV7_PSCI
obj-y  += psci.o ls102xa_psci.o
endif
