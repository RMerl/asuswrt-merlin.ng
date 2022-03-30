// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 */

#include <common.h>
#include <linux/errno.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc-regs.h"

#undef DPLL_SSC_RATE_1PER

int uniphier_ld4_dpll_init(const struct uniphier_board_data *bd)
{
	unsigned int dram_freq = bd->dram_freq;
	u32 tmp;

	/*
	 * Set Frequency
	 * Set 0xc(1600MHz)/0xd(1333MHz)/0xe(1066MHz)
	 * to FOUT (DPLLCTRL.bit[29:20])
	 */
	tmp = readl(SC_DPLLCTRL);
	tmp &= ~0x000f0000;
	switch (dram_freq) {
	case 1333:
		tmp |= 0x000d0000;
		break;
	case 1600:
		tmp |= 0x000c0000;
		break;
	default:
		pr_err("Unsupported frequency");
		return -EINVAL;
	}

#if defined(DPLL_SSC_RATE_1PER)
	tmp &= ~SC_DPLLCTRL_SSC_RATE;
#else
	tmp |= SC_DPLLCTRL_SSC_RATE;
#endif
	writel(tmp, SC_DPLLCTRL);

	tmp = readl(SC_DPLLCTRL2);
	tmp |= SC_DPLLCTRL2_NRSTDS;
	writel(tmp, SC_DPLLCTRL2);

	/* Wait 500 usec until dpll gets stable */
	udelay(500);

	return 0;
}
