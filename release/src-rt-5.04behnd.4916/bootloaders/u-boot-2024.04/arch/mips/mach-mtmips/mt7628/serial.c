// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/io.h>
#include "mt7628.h"

void mtmips_spl_serial_init(void)
{
#ifdef CONFIG_SPL_SERIAL
	void __iomem *base = ioremap_nocache(SYSCTL_BASE, SYSCTL_SIZE);

#if CONFIG_CONS_INDEX == 1
	clrbits_32(base + SYSCTL_GPIO_MODE1_REG, UART0_MODE_M);
#elif CONFIG_CONS_INDEX == 2
	clrbits_32(base + SYSCTL_GPIO_MODE1_REG, UART1_MODE_M);
#elif CONFIG_CONS_INDEX == 3
	setbits_32(base + SYSCTL_AGPIO_CFG_REG, EPHY_GPIO_AIO_EN_M);
#ifdef CONFIG_SPL_UART2_SPIS_PINMUX
	setbits_32(base + SYSCTL_GPIO_MODE1_REG, SPIS_MODE_M);
	clrsetbits_32(base + SYSCTL_GPIO_MODE1_REG, UART2_MODE_M,
		      1 << UART2_MODE_S);
#else
	clrbits_32(base + SYSCTL_GPIO_MODE1_REG, UART2_MODE_M);
	clrsetbits_32(base + SYSCTL_GPIO_MODE1_REG, SPIS_MODE_M,
		      1 << SPIS_MODE_S);
#endif /* CONFIG_SPL_UART2_SPIS_PINMUX */
#endif /* CONFIG_CONS_INDEX */
#endif /* CONFIG_SPL_SERIAL */
}
