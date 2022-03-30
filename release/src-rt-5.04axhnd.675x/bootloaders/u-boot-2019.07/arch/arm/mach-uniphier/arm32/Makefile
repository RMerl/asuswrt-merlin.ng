# SPDX-License-Identifier: GPL-2.0+

ifdef CONFIG_SPL_BUILD
obj-y += lowlevel_init.o
obj-$(CONFIG_DEBUG_LL) += debug_ll.o
else
obj-y += late_lowlevel_init.o
obj-y += cache-uniphier.o
obj-$(CONFIG_ARMV7_PSCI) += psci.o psci_smp.o
endif

obj-y += timer.o
