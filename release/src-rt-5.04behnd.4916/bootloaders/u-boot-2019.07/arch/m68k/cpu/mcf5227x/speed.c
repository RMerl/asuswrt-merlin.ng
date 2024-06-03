// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <asm/processor.h>

#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Low Power Divider specifications
 */
#define CLOCK_LPD_MIN		(1 << 0)	/* Divider (decoded) */
#define CLOCK_LPD_MAX		(1 << 15)	/* Divider (decoded) */

#define CLOCK_PLL_FVCO_MAX	540000000
#define CLOCK_PLL_FVCO_MIN	300000000

#define CLOCK_PLL_FSYS_MAX	266666666
#define CLOCK_PLL_FSYS_MIN	100000000
#define MHZ			1000000

void clock_enter_limp(int lpdiv)
{
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	int i, j;

	/* Check bounds of divider */
	if (lpdiv < CLOCK_LPD_MIN)
		lpdiv = CLOCK_LPD_MIN;
	if (lpdiv > CLOCK_LPD_MAX)
		lpdiv = CLOCK_LPD_MAX;

	/* Round divider down to nearest power of two */
	for (i = 0, j = lpdiv; j != 1; j >>= 1, i++) ;

	/* Apply the divider to the system clock */
	clrsetbits_be16(&ccm->cdr, 0x0f00, CCM_CDR_LPDIV(i));

	/* Enable Limp Mode */
	setbits_be16(&ccm->misccr, CCM_MISCCR_LIMP);
}

/*
 * brief   Exit Limp mode
 * warning The PLL should be set and locked prior to exiting Limp mode
 */
void clock_exit_limp(void)
{
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	pll_t *pll = (pll_t *)MMAP_PLL;

	/* Exit Limp mode */
	clrbits_be16(&ccm->misccr, CCM_MISCCR_LIMP);

	/* Wait for the PLL to lock */
	while (!(in_be32(&pll->psr) & PLL_PSR_LOCK))
		;
}

/*
 * get_clocks() fills in gd->cpu_clock and gd->bus_clk
 */
int get_clocks(void)
{

	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	pll_t *pll = (pll_t *)MMAP_PLL;
	int vco, temp, pcrvalue, pfdr;
	u8 bootmode;

	pcrvalue = in_be32(&pll->pcr) & 0xFF0F0FFF;
	pfdr = pcrvalue >> 24;

	if (pfdr == 0x1E)
		bootmode = 0;	/* Normal Mode */

#ifdef CONFIG_CF_SBF
	bootmode = 3;		/* Serial Mode */
#endif

	if (bootmode == 0) {
		/* Normal mode */
		vco = ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		if ((vco < CLOCK_PLL_FVCO_MIN) || (vco > CLOCK_PLL_FVCO_MAX)) {
			/* Default value */
			pcrvalue = (in_be32(&pll->pcr) & 0x00FFFFFF);
			pcrvalue |= 0x1E << 24;
			out_be32(&pll->pcr, pcrvalue);
			vco =
			    ((in_be32(&pll->pcr) & 0xFF000000) >> 24) *
			    CONFIG_SYS_INPUT_CLKSRC;
		}
		gd->arch.vco_clk = vco;	/* Vco clock */
	} else if (bootmode == 3) {
		/* serial mode */
		vco = ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		gd->arch.vco_clk = vco;	/* Vco clock */
	}

	if ((in_be16(&ccm->ccr) & CCM_MISCCR_LIMP) == CCM_MISCCR_LIMP) {
		/* Limp mode */
	} else {
		gd->arch.inp_clk = CONFIG_SYS_INPUT_CLKSRC; /* Input clock */

		temp = (in_be32(&pll->pcr) & PLL_PCR_OUTDIV1_MASK) + 1;
		gd->cpu_clk = vco / temp;	/* cpu clock */

		temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
		gd->arch.flb_clk = vco / temp;	/* flexbus clock */
		gd->bus_clk = gd->arch.flb_clk;
	}

#ifdef CONFIG_SYS_I2C_FSL
	gd->arch.i2c1_clk = gd->bus_clk;
#endif

	return (0);
}
