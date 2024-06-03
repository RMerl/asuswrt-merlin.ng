# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2010,2011 Nvidia Corporation.

ifdef CONFIG_SPL_BUILD
obj-y	+= cpu.o
endif

# The AVP is ARMv4T architecture so we must use special compiler
# flags for any startup files it might use.
CFLAGS_warmboot_avp.o = -march=armv4t -U__LINUX_ARM_ARCH__ \
	-D__LINUX_ARM_ARCH__=4

obj-y	+= clock.o funcmux.o pinmux.o
obj-$(CONFIG_TEGRA_LP0) += warmboot.o crypto.o warmboot_avp.o
obj-$(CONFIG_TEGRA_CLOCK_SCALING) += emc.o
obj-$(CONFIG_TEGRA_PMU) += pmu.o
