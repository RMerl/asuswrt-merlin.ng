// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/io.h>
#include "../mt7621.h"

void mtmips_spl_serial_init(void)
{
#ifdef CONFIG_SPL_SERIAL
	void __iomem *base = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

#if CONFIG_CONS_INDEX == 1
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UART1_MODE);
#elif CONFIG_CONS_INDEX == 2
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UART2_MODE_M);
#elif CONFIG_CONS_INDEX == 3
	clrbits_32(base + SYSCTL_GPIOMODE_REG, UART3_MODE_M);
#endif /* CONFIG_CONS_INDEX */
#endif /* CONFIG_SPL_SERIAL */
}
