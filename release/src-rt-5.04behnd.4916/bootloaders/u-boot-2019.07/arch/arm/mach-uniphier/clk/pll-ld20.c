// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/delay.h>

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

/* PLL type: SSC */
#define SC_CPLLCTRL	(SC_BASE_ADDR | 0x1400)	/* CPU/ARM */
#define SC_SPLLCTRL	(SC_BASE_ADDR | 0x1410)	/* misc */
#define SC_SPLL2CTRL	(SC_BASE_ADDR | 0x1420)	/* DSP */
#define SC_MPLLCTRL	(SC_BASE_ADDR | 0x1430)	/* Video codec */
#define SC_VPPLLCTRL	(SC_BASE_ADDR | 0x1440)	/* VPE etc. */
#define SC_GPPLLCTRL	(SC_BASE_ADDR | 0x1450)	/* GPU/Mali */
#define SC_DPLL0CTRL	(SC_BASE_ADDR | 0x1460)	/* DDR memory 0 */
#define SC_DPLL1CTRL	(SC_BASE_ADDR | 0x1470)	/* DDR memory 1 */
#define SC_DPLL2CTRL	(SC_BASE_ADDR | 0x1480)	/* DDR memory 2 */

/* PLL type: VPLL27 */
#define SC_VPLL27FCTRL	(SC_BASE_ADDR | 0x1500)
#define SC_VPLL27ACTRL	(SC_BASE_ADDR | 0x1520)

/* PLL type: DSPLL */
#define SC_VPLL8KCTRL	(SC_BASE_ADDR | 0x1540)
#define SC_A2PLLCTRL	(SC_BASE_ADDR | 0x15C0)

void uniphier_ld20_pll_init(void)
{
	uniphier_ld20_sscpll_init(SC_CPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	/* do nothing for SPLL */
	uniphier_ld20_sscpll_init(SC_SPLL2CTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	uniphier_ld20_sscpll_init(SC_MPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_VPPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	uniphier_ld20_sscpll_init(SC_GPPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	mdelay(1);

	uniphier_ld20_sscpll_ssc_en(SC_CPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_SPLL2CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_MPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_VPPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_GPPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLL0CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLL1CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLL2CTRL);

	uniphier_ld20_vpll27_init(SC_VPLL27FCTRL);
	uniphier_ld20_vpll27_init(SC_VPLL27ACTRL);

	uniphier_ld20_dspll_init(SC_VPLL8KCTRL);
	uniphier_ld20_dspll_init(SC_A2PLLCTRL);
}
