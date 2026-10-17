// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/io.h>
#include <asm/addrspace.h>
#include "mt7620.h"

void board_debug_uart_init(void)
{
	void __iomem *base = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

#if CONFIG_DEBUG_UART_BASE == 0xb0000500 /* KSEG1ADDR(UARTFULL_BASE) */
	clrsetbits_32(base + SYSCTL_GPIOMODE_REG, UARTF_SHARE_MODE_M,
		      UARTF_MODE_UARTF_GPIO << UARTF_SHARE_MODE_S);
#else
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UARTL_GPIO_MODE);
#endif
}

void mtmips_spl_serial_init(void)
{
#ifdef CONFIG_SPL_SERIAL
	void __iomem *base = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

#if CONFIG_CONS_INDEX == 1
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UARTL_GPIO_MODE);
#elif CONFIG_CONS_INDEX == 2
	clrsetbits_32(base + SYSCTL_GPIOMODE_REG, UARTF_SHARE_MODE_M,
		      UARTF_MODE_UARTF_GPIO << UARTF_SHARE_MODE_S);
#endif
#endif /* CONFIG_SPL_SERIAL */
}
