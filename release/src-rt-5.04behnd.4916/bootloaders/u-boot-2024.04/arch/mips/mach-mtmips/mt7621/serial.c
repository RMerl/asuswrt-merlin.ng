// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/io.h>
#include <asm/addrspace.h>
#include "mt7621.h"

void board_debug_uart_init(void)
{
	void __iomem *base = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

#if CONFIG_DEBUG_UART_BASE == 0xbe000c00 /* KSEG1ADDR(UART1_BASE) */
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UART1_MODE);
#elif CONFIG_DEBUG_UART_BASE == 0xbe000d00 /* KSEG1ADDR(UART2_BASE) */
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UART2_MODE_M);
#elif CONFIG_DEBUG_UART_BASE == 0xbe000e00 /* KSEG1ADDR(UART3_BASE) */
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UART3_MODE_M);
#endif
}
