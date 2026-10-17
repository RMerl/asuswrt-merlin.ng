// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Socionext Inc.
 */

#include <linux/delay.h>

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

/* PLL type: SSC */
#define SC_CPLLCTRL	0x1400	/* CPU/ARM */
#define SC_SPLLCTRL	0x1410	/* misc */
#define SC_SPLL2CTRL	0x1420	/* DSP */
#define SC_VPPLLCTRL	0x1430	/* VPE */
#define SC_VGPLLCTRL	0x1440
#define SC_DECPLLCTRL	0x1450
#define SC_ENCPLLCTRL	0x1460
#define SC_PXFPLLCTRL	0x1470
#define SC_DPLL0CTRL	0x1480	/* DDR memory 0 */
#define SC_DPLL1CTRL	0x1490	/* DDR memory 1 */
#define SC_DPLL2CTRL	0x14a0	/* DDR memory 2 */
#define SC_VSPLLCTRL	0x14c0

/* PLL type: VPLL27 */
#define SC_VPLL27FCTRL	0x1500
#define SC_VPLL27ACTRL	0x1520

/* PLL type: DSPLL */
#define SC_VPLL8KCTRL	0x1540

void uniphier_pxs3_pll_init(void)
{
	uniphier_ld20_sscpll_init(SC_CPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	/* do nothing for SPLL */
	uniphier_ld20_sscpll_init(SC_SPLL2CTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	uniphier_ld20_sscpll_init(SC_VPPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_VGPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_DECPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_ENCPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 4);
	uniphier_ld20_sscpll_init(SC_PXFPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);
	uniphier_ld20_sscpll_init(SC_VSPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	mdelay(1);

	uniphier_ld20_sscpll_ssc_en(SC_CPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_SPLL2CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_VPPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_VGPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DECPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_ENCPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_PXFPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLL0CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLL1CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLL2CTRL);
	uniphier_ld20_sscpll_ssc_en(SC_VSPLLCTRL);

	uniphier_ld20_vpll27_init(SC_VPLL27FCTRL);
	uniphier_ld20_vpll27_init(SC_VPLL27ACTRL);

	uniphier_ld20_dspll_init(SC_VPLL8KCTRL);
}
