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

#ifdef CONFIG_MCF5445x
	/* Apply the divider to the system clock */
	clrsetbits_be16(&ccm->cdr, 0x0f00, CCM_CDR_LPDIV(i));
#endif

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

#ifdef CONFIG_MCF5445x
void setup_5445x_clocks(void)
{
	ccm_t *ccm = (ccm_t *)MMAP_CCM;
	pll_t *pll = (pll_t *)MMAP_PLL;
	int pllmult_nopci[] = { 20, 10, 24, 18, 12, 6, 16, 8 };
	int pllmult_pci[] = { 12, 6, 16, 8 };
	int vco = 0, temp, fbtemp, pcrvalue;
	int *pPllmult = NULL;
	u16 fbpll_mask;
#ifdef CONFIG_PCI
	int bPci;
#endif

#ifdef CONFIG_M54455EVB
	u8 *cpld = (u8 *)(CONFIG_SYS_CS2_BASE + 3);
#endif
	u8 bootmode;

	/* To determine PCI is present or not */
	if (((in_be16(&ccm->ccr) & CCM_CCR_360_FBCONFIG_MASK) == 0x00e0) ||
	    ((in_be16(&ccm->ccr) & CCM_CCR_360_FBCONFIG_MASK) == 0x0060)) {
		pPllmult = &pllmult_pci[0];
		fbpll_mask = 3;		/* 11b */
#ifdef CONFIG_PCI
		bPci = 1;
#endif
	} else {
		pPllmult = &pllmult_nopci[0];
		fbpll_mask = 7;		/* 111b */
#ifdef CONFIG_PCI
		gd->pci_clk = 0;
		bPci = 0;
#endif
	}

#ifdef CONFIG_M54455EVB
	bootmode = (in_8(cpld) & 0x03);

	if (bootmode != 3) {
		/* Temporary read from CCR- fixed fb issue, must be the same clock
		   as pci or input clock, causing cpld/fpga read inconsistancy */
		fbtemp = pPllmult[ccm->ccr & fbpll_mask];

		/* Break down into small pieces, code still in flex bus */
		pcrvalue = in_be32(&pll->pcr) & 0xFFFFF0FF;
		temp = fbtemp - 1;
		pcrvalue |= PLL_PCR_OUTDIV3(temp);

		out_be32(&pll->pcr, pcrvalue);
	}
#endif
#ifdef CONFIG_M54451EVB
	/* No external logic to read the bootmode, hard coded from built */
#ifdef CONFIG_CF_SBF
	bootmode = 3;
#else
	bootmode = 2;

	/* default value is 16 mul, set to 20 mul */
	pcrvalue = (in_be32(&pll->pcr) & 0x00FFFFFF) | 0x14000000;
	out_be32(&pll->pcr, pcrvalue);
	while ((in_be32(&pll->psr) & PLL_PSR_LOCK) != PLL_PSR_LOCK)
		;
#endif
#endif

	if (bootmode == 0) {
		/* RCON mode */
		vco = pPllmult[ccm->rcon & fbpll_mask] * CONFIG_SYS_INPUT_CLKSRC;

		if ((vco < CLOCK_PLL_FVCO_MIN) || (vco > CLOCK_PLL_FVCO_MAX)) {
			/* invaild range, re-set in PCR */
			int temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
			int i, j, bus;

			j = (in_be32(&pll->pcr) & 0xFF000000) >> 24;
			for (i = j; i < 0xFF; i++) {
				vco = i * CONFIG_SYS_INPUT_CLKSRC;
				if (vco >= CLOCK_PLL_FVCO_MIN) {
					bus = vco / temp;
					if (bus <= CLOCK_PLL_FSYS_MIN - MHZ)
						continue;
					else
						break;
				}
			}
			pcrvalue = in_be32(&pll->pcr) & 0x00FF00FF;
			fbtemp = ((i - 1) << 8) | ((i - 1) << 12);
			pcrvalue |= ((i << 24) | fbtemp);

			out_be32(&pll->pcr, pcrvalue);
		}
		gd->arch.vco_clk = vco;	/* Vco clock */
	} else if (bootmode == 2) {
		/* Normal mode */
		vco =  ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		if ((vco < CLOCK_PLL_FVCO_MIN) || (vco > CLOCK_PLL_FVCO_MAX)) {
			/* Default value */
			pcrvalue = (in_be32(&pll->pcr) & 0x00FFFFFF);
			pcrvalue |= pPllmult[in_be16(&ccm->ccr) & fbpll_mask] << 24;
			out_be32(&pll->pcr, pcrvalue);
			vco = ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		}
		gd->arch.vco_clk = vco;	/* Vco clock */
	} else if (bootmode == 3) {
		/* serial mode */
		vco =  ((in_be32(&pll->pcr) & 0xFF000000) >> 24) * CONFIG_SYS_INPUT_CLKSRC;
		gd->arch.vco_clk = vco;	/* Vco clock */
	}

	if ((in_be16(&ccm->ccr) & CCM_MISCCR_LIMP) == CCM_MISCCR_LIMP) {
		/* Limp mode */
	} else {
		gd->arch.inp_clk = CONFIG_SYS_INPUT_CLKSRC; /* Input clock */

		temp = (in_be32(&pll->pcr) & PLL_PCR_OUTDIV1_MASK) + 1;
		gd->cpu_clk = vco / temp;	/* cpu clock */

		temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV2_MASK) >> 4) + 1;
		gd->bus_clk = vco / temp;	/* bus clock */

		temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV3_MASK) >> 8) + 1;
		gd->arch.flb_clk = vco / temp;	/* FlexBus clock */

#ifdef CONFIG_PCI
		if (bPci) {
			temp = ((in_be32(&pll->pcr) & PLL_PCR_OUTDIV4_MASK) >> 12) + 1;
			gd->pci_clk = vco / temp;	/* PCI clock */
		}
#endif
	}

#ifdef CONFIG_SYS_I2C_FSL
	gd->arch.i2c1_clk = gd->bus_clk;
#endif
}
#endif

/* get_clocks() fills in gd->cpu_clock and gd->bus_clk */
int get_clocks(void)
{
#ifdef CONFIG_MCF5441x
	setup_5441x_clocks();
#endif
#ifdef CONFIG_MCF5445x
	setup_5445x_clocks();
#endif

#ifdef CONFIG_SYS_FSL_I2C
	gd->arch.i2c1_clk = gd->bus_clk;
#endif

	return (0);
}
