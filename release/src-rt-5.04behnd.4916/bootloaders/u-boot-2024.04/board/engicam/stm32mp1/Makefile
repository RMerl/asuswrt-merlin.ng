# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright (C) 2018, STMicroelectronics - All Rights Reserved
#

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y += stm32mp1.o
endif

obj-$(CONFIG_DEBUG_UART_BOARD_INIT) += ../../st/stm32mp1/debug_uart.o
