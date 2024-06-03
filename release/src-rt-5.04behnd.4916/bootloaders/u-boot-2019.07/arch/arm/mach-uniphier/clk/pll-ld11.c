// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 */

#include <linux/delay.h>
#include <linux/io.h>

#include "../init.h"
#include "../sc64-regs.h"
#include "pll.h"

/* PLL type: SSC */
#define SC_CPLLCTRL	(SC_BASE_ADDR | 0x1400)	/* CPU/ARM */
#define SC_SPLLCTRL	(SC_BASE_ADDR | 0x1410)	/* misc */
#define SC_MPLLCTRL	(SC_BASE_ADDR | 0x1430)	/* DSP */
#define SC_VSPLLCTRL	(SC_BASE_ADDR | 0x1440)	/* Video codec, VPE etc. */
#define SC_DPLLCTRL	(SC_BASE_ADDR | 0x1460)	/* DDR memory */

/* PLL type: VPLL27 */
#define SC_VPLL27FCTRL	(SC_BASE_ADDR | 0x1500)
#define SC_VPLL27ACTRL	(SC_BASE_ADDR | 0x1520)

void uniphier_ld11_pll_init(void)
{
	uniphier_ld20_sscpll_init(SC_CPLLCTRL, 1960, 1, 2);	/* 2000MHz -> 1960MHz */
	/* do nothing for SPLL */
	uniphier_ld20_sscpll_init(SC_MPLLCTRL, 1600, 1, 2);	/* 1500MHz -> 1600MHz */
	uniphier_ld20_sscpll_init(SC_VSPLLCTRL, UNIPHIER_PLL_FREQ_DEFAULT, 0, 2);

	uniphier_ld20_sscpll_set_regi(SC_MPLLCTRL, 5);

	mdelay(1);

	uniphier_ld20_sscpll_ssc_en(SC_CPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_MPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_VSPLLCTRL);
	uniphier_ld20_sscpll_ssc_en(SC_DPLLCTRL);

	uniphier_ld20_vpll27_init(SC_VPLL27FCTRL);
	uniphier_ld20_vpll27_init(SC_VPLL27ACTRL);

	writel(0, SC_CA53_GEARSET);	/* Gear0: CPLL/2 */
	writel(SC_CA_GEARUPD, SC_CA53_GEARUPD);
}
