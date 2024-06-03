# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright 2004 Freescale Semiconductor, Inc.

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

extra-y	= start.o

ifdef MINIMAL

obj-y	+= spl_minimal.o

else

obj-y += traps.o
obj-y += cpu.o
obj-y += cpu_init.o
obj-y += speed.o
obj-y += interrupts.o
obj-y += ecc.o
obj-$(CONFIG_QE) += qe_io.o
obj-$(CONFIG_FSL_SERDES) += serdes.o
ifndef CONFIG_ARCH_MPC8308
obj-$(CONFIG_PCI) += pci.o
endif
obj-$(CONFIG_PCIE) += pcie.o
obj-$(CONFIG_OF_LIBFDT) += fdt.o

ifndef CONFIG_SYS_FSL_DDRC_GEN2
obj-y += spd_sdram.o
endif
obj-$(CONFIG_SYS_FSL_DDR2) += law.o

endif # not minimal
