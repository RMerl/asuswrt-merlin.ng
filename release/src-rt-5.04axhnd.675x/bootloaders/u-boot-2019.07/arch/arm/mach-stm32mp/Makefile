# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

obj-y += cpu.o
obj-y += dram_init.o
obj-y += syscon.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y += bsec.o
ifndef CONFIG_STM32MP1_TRUSTED
obj-$(CONFIG_SYSRESET) += cmd_poweroff.o
endif
endif
obj-$(CONFIG_ARMV7_PSCI) += psci.o
obj-$(CONFIG_$(SPL_)DM_REGULATOR) += pwr_regulator.o
