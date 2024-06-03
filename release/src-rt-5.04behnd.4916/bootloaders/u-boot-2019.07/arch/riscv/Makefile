# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2017 Andes Technology Corporation.
# Rick Chen, Andes Technology Corporation <rick@andestech.com>

ifeq ($(CONFIG_ARCH_RV64I),y)
	ARCH_BASE = rv64im
	ABI = lp64
endif
ifeq ($(CONFIG_ARCH_RV32I),y)
	ARCH_BASE = rv32im
	ABI = ilp32
endif
ifeq ($(CONFIG_RISCV_ISA_A),y)
	ARCH_A = a
endif
ifeq ($(CONFIG_RISCV_ISA_C),y)
	ARCH_C = c
endif
ifeq ($(CONFIG_CMODEL_MEDLOW),y)
	CMODEL = medlow
endif
ifeq ($(CONFIG_CMODEL_MEDANY),y)
	CMODEL = medany
endif

ARCH_FLAGS = -march=$(ARCH_BASE)$(ARCH_A)$(ARCH_C) -mabi=$(ABI) \
	     -mcmodel=$(CMODEL)

PLATFORM_CPPFLAGS	+= $(ARCH_FLAGS)
CFLAGS_EFI		+= $(ARCH_FLAGS)

head-y := arch/riscv/cpu/start.o

libs-y += arch/riscv/cpu/
libs-y += arch/riscv/cpu/$(CPU)/
libs-y += arch/riscv/lib/
