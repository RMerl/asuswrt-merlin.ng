// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2023 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <clock_legacy.h>
#include <asm/arch/renesas.h>
#include <asm/io.h>

#define CPGWPR  0xE6150900
#define CPGWPCR	0xE6150904

/* PLL */
#define PLL0CR		0xE61500D8
#define PLL0_STC_MASK	0x7F000000
#define PLL0_STC_OFFSET	24

#define CLK2MHZ(clk)	(clk / 1000 / 1000)
void s_init(void)
{
	struct rcar_rwdt *rwdt = (struct rcar_rwdt *)RWDT_BASE;
	struct rcar_swdt *swdt = (struct rcar_swdt *)SWDT_BASE;
	u32 stc;

	/* Watchdog init */
	writel(0xA5A5A500, &rwdt->rwtcsra);
	writel(0xA5A5A500, &swdt->swtcsra);

	/* CPU frequency setting. Set to 0.8GHz */
	stc = ((800 / CLK2MHZ(get_board_sys_clk())) - 1) << PLL0_STC_OFFSET;
	clrsetbits_le32(PLL0CR, PLL0_STC_MASK, stc);
}

int board_early_init_f(void)
{
	/* Unlock CPG access */
	writel(0xA5A5FFFF, CPGWPR);
	writel(0x5A5A0000, CPGWPCR);

	return 0;
}
