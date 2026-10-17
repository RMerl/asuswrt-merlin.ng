// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <clock_legacy.h>
#include <asm/global_data.h>
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

#ifdef CONFIG_MCF5441x
void setup_5441x_clocks(void)
{
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	pll_t *pll = (pll_t *)MMAP_PLL;
	int temp, vco = 0, bootmod_ccr, pdr;

	bootmod_ccr = (in_be16(&ccm->ccr) & CCM_CCR_BOOTMOD) >> 14;

	switch (bootmod_ccr) {
	case 0:
		out_be32(&pll->pcr, 0x00000013);
		out_be32(&pll->pdr, 0x00e70c61);
		clock_exit_limp();
		break;
	case 2:
		break;
	case 3:
		break;
	}

	/*Change frequency for Modelo SER1 USB host*/
#ifdef CONFIG_LOW_MCFCLK
	temp = in_be32(&pll->pcr);
	temp &= ~0x3f;
	temp |= 5;
	out_be32(&pll->pcr, temp);

	temp = in_be32(&pll->pdr);
	temp &= ~0x001f0000;
	temp |= 0x00040000;
	out_be32(&pll->pdr, temp);
	__asm__("tpf");
#endif

	setbits_be16(&ccm->misccr2, 0x02);

	vco =  ((in_be32(&pll->pcr) & PLL_CR_FBKDIV_BITS) + 1) *
		CONFIG_SYS_INPUT_CLKSRC;
	gd->arch.vco_clk = vco;

	gd->arch.inp_clk = CONFIG_SYS_INPUT_CLKSRC;	/* Input clock */

	pdr = in_be32(&pll->pdr);
	temp = (pdr & PLL_DR_OUTDIV1_BITS) + 1;
	gd->cpu_clk = vco / temp;	/* cpu clock */
	gd->arch.flb_clk = vco / temp;	/* FlexBus clock */
	gd->arch.flb_clk >>= 1;
	if (in_be16(&ccm->misccr2) & 2)		/* fsys/4 */
		gd->arch.flb_clk >>= 1;

	temp = ((pdr & PLL_DR_OUTDIV2_BITS) >> 5) + 1;
	gd->bus_clk = vco / temp;	/* bus clock */

	temp = ((pdr & PLL_DR_OUTDIV3_BITS) >> 10) + 1;
	gd->arch.sdhc_clk = vco / temp;
}
#endif

/* get_clocks() fills in gd->cpu_clock and gd->bus_clk */
int get_clocks(void)
{
#ifdef CONFIG_MCF5441x
	setup_5441x_clocks();
#endif

	if (IS_ENABLED(CONFIG_SYS_I2C_FSL))
		gd->arch.i2c1_clk = gd->bus_clk;

	return (0);
}
