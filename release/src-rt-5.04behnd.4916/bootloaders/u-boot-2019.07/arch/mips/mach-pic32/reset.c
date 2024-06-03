// SPDX-License-Identifier: GPL-2.0+
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <mach/pic32.h>

/* SYSKEY */
#define UNLOCK_KEY1	0xaa996655
#define UNLOCK_KEY2	0x556699aa
#define LOCK_KEY	0

#define RSWRST          0x1250

void _machine_restart(void)
{
	void __iomem *base;

	base = pic32_get_syscfg_base();

	/* unlock sequence */
	writel(LOCK_KEY, base + SYSKEY);
	writel(UNLOCK_KEY1, base + SYSKEY);
	writel(UNLOCK_KEY2, base + SYSKEY);

	/* soft reset */
	writel(0x1, base + RSWRST);
	(void) readl(base + RSWRST);

	while (1)
		;
}
