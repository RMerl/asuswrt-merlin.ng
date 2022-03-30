# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2010-2019 Nvidia Corporation.
#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-y += cpu.o
else
obj-$(CONFIG_CMD_ENTERRCM) += cmd_enterrcm.o
endif

obj-$(CONFIG_TEGRA_GP_PADCTRL) += ap.o
obj-y += board.o board2.o
obj-y += cache.o
obj-$(CONFIG_TEGRA_CLKRST) += clock.o
obj-$(CONFIG_TEGRA_PINCTRL) += pinmux-common.o
obj-$(CONFIG_TEGRA_PMC) += powergate.o
obj-y += xusb-padctl-dummy.o

obj-$(CONFIG_ARM64) += arm64-mmu.o cboot.o
obj-y += dt-setup.o
obj-$(CONFIG_TEGRA_CLOCK_SCALING) += emc.o
obj-$(CONFIG_TEGRA_GPU) += gpu.o
obj-$(CONFIG_TEGRA_IVC) += ivc.o
ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ARMV7_PSCI) += psci.o
endif
obj-$(CONFIG_DISPLAY_CPUINFO) += sys_info.o
obj-y += pmc.o

obj-$(CONFIG_TEGRA20) += tegra20/
obj-$(CONFIG_TEGRA30) += tegra30/
obj-$(CONFIG_TEGRA114) += tegra114/
obj-$(CONFIG_TEGRA124) += tegra124/
obj-$(CONFIG_TEGRA186) += tegra186/
obj-$(CONFIG_TEGRA210) += tegra210/
