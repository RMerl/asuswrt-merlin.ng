// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 */

#include <linux/delay.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

int uniphier_sld8_dpll_init(const struct uniphier_board_data *bd)
{
	u32 tmp;
	/*
	 * Set DPLL SSC parameters for DPLLCTRL3
	 * [23]    DIVN_TEST    0x1
	 * [22:16] DIVN         0x50
	 * [10]    FREFSEL_TEST 0x1
	 * [9:8]   FREFSEL      0x2
	 * [4]     ICPD_TEST    0x1
	 * [3:0]   ICPD         0xb
	 */
	tmp = readl(SC_DPLLCTRL3);
	tmp &= ~0x00ff0717;
	tmp |= 0x00d0061b;
	writel(tmp, SC_DPLLCTRL3);

	/*
	 * Set DPLL SSC parameters for DPLLCTRL
	 *                    <-1%>          <-2%>
	 * [29:20] SSC_UPCNT 132 (0x084)    132  (0x084)
	 * [14:0]  SSC_dK    6335(0x18bf)   12710(0x31a6)
	 */
	tmp = readl(SC_DPLLCTRL);
	tmp &= ~0x3ff07fff;
#ifdef DPLL_SSC_RATE_1PER
	tmp |= 0x084018bf;
#else
	tmp |= 0x084031a6;
#endif
	writel(tmp, SC_DPLLCTRL);

	/*
	 * Set DPLL SSC parameters for DPLLCTRL2
	 * [31:29]  SSC_STEP     0
	 * [27]     SSC_REG_REF  1
	 * [26:20]  SSC_M        79     (0x4f)
	 * [19:0]   SSC_K        964689 (0xeb851)
	 */
	tmp = readl(SC_DPLLCTRL2);
	tmp &= ~0xefffffff;
	tmp |= 0x0cfeb851;
	writel(tmp, SC_DPLLCTRL2);

	/* Wait 500 usec until dpll gets stable */
	udelay(500);

	return 0;
}
