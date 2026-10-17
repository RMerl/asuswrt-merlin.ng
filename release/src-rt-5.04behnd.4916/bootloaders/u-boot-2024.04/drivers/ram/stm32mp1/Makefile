# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

obj-y += stm32mp1_ram.o
obj-y += stm32mp1_ddr.o

obj-$(CONFIG_STM32MP1_DDR_INTERACTIVE) += stm32mp1_interactive.o
obj-$(CONFIG_STM32MP1_DDR_TESTS) += stm32mp1_tests.o

ifneq ($(DDR_INTERACTIVE),)
CFLAGS_stm32mp1_interactive.o += -DCONFIG_STM32MP1_DDR_INTERACTIVE_FORCE=y
endif
