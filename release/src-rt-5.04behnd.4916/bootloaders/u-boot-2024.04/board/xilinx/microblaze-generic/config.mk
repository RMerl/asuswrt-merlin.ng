# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2007 - 2016 Michal Simek
#
# Michal SIMEK <monstr@monstr.eu>

CPU_VER := $(shell echo $(CONFIG_XILINX_MICROBLAZE0_HW_VER))

# USE_HW_MUL can be 0, 1, or 2, defining a hierarchy of HW Mul support.
CPUFLAGS-$(subst 1,,$(CONFIG_XILINX_MICROBLAZE0_USE_HW_MUL)) += -mxl-multiply-high
CPUFLAGS-$(CONFIG_XILINX_MICROBLAZE0_USE_HW_MUL) += -mno-xl-soft-mul
CPUFLAGS-$(CONFIG_XILINX_MICROBLAZE0_USE_DIV) += -mno-xl-soft-div
CPUFLAGS-$(CONFIG_XILINX_MICROBLAZE0_USE_BARREL) += -mxl-barrel-shift
CPUFLAGS-$(CONFIG_XILINX_MICROBLAZE0_USE_PCMP_INSTR) += -mxl-pattern-compare

CPUFLAGS-1 += $(call cc-option,-mcpu=v$(CPU_VER))

PLATFORM_CPPFLAGS += $(CPUFLAGS-1) $(CPUFLAGS-2)
