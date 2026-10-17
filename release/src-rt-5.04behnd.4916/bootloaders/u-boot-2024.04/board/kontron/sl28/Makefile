# SPDX-License-Identifier:	GPL-2.0+

ifndef CONFIG_SPL_BUILD
obj-y += sl28.o cmds.o
endif

obj-y += common.o ddr.o

obj-$(CONFIG_ARMV8_PSCI) += psci.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_SPL_ATF) += spl_atf.o
endif
