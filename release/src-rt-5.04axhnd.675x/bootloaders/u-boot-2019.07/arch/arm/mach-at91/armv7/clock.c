// SPDX-License-Identifier: GPL-2.0+
/*
 * [origin: Linux kernel linux/arch/arm/mach-at91/clock.c]
 *
 * Copyright (C) 2005 David Brownell
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 * Copyright (C) 2013 Bo Shen <voice.shen@atmel.com>
 * Copyright (C) 2015 Wenyou Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>

#if !defined(CONFIG_AT91FAMILY)
# error You need to define CONFIG_AT91FAMILY in your board config!
#endif

DECLARE_GLOBAL_DATA_PTR;

static unsigned long at91_css_to_rate(unsigned long css)
{
	switch (css) {
	case AT91_PMC_MCKR_CSS_SLOW:
		return CONFIG_SYS_AT91_SLOW_CLOCK;
	case AT91_PMC_MCKR_CSS_MAIN:
		return gd->arch.main_clk_rate_hz;
	case AT91_PMC_MCKR_CSS_PLLA:
		return gd->arch.plla_rate_hz;
	}

	return 0;
}

static u32 at91_pll_rate(u32 freq, u32 reg)
{
	unsigned mul, div;

	div = reg & 0xff;
	mul = (reg >> 18) & 0x7f;
	if (div && mul) {
		freq /= div;
		freq *= mul + 1;
	} else {
		freq = 0;
	}

	return freq;
}

int at91_clock_init(unsigned long main_clock)
{
	unsigned freq, mckr;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
#ifndef CONFIG_SYS_AT91_MAIN_CLOCK
	unsigned tmp;
	/*
	 * When the bootloader initialized the main oscillator correctly,
	 * there's no problem using the cycle counter.  But if it didn't,
	 * or when using oscillator bypass mode, we must be told the speed
	 * of the main clock.
	 */
	if (!main_clock) {
		do {
			tmp = readl(&pmc->mcfr);
		} while (!(tmp & AT91_PMC_MCFR_MAINRDY));
		tmp &= AT91_PMC_MCFR_MAINF_MASK;
		main_clock = tmp * (CONFIG_SYS_AT91_SLOW_CLOCK / 16);
	}
#endif
	gd->arch.main_clk_rate_hz = main_clock;

	/* report if PLLA is more than mildly overclocked */
	gd->arch.plla_rate_hz = at91_pll_rate(main_clock, readl(&pmc->pllar));

	/*
	 * MCK and CPU derive from one of those primary clocks.
	 * For now, assume this parentage won't change.
	 */
	mckr = readl(&pmc->mckr);

	/* plla divisor by 2 */
	if (mckr & (1 << 12))
		gd->arch.plla_rate_hz >>= 1;

	gd->arch.mck_rate_hz = at91_css_to_rate(mckr & AT91_PMC_MCKR_CSS_MASK);
	freq = gd->arch.mck_rate_hz;

	/* prescale */
	freq >>= mckr & AT91_PMC_MCKR_PRES_MASK;

	switch (mckr & AT91_PMC_MCKR_MDIV_MASK) {
	case AT91_PMC_MCKR_MDIV_2:
		gd->arch.mck_rate_hz = freq / 2;
		break;
	case AT91_PMC_MCKR_MDIV_3:
		gd->arch.mck_rate_hz = freq / 3;
		break;
	case AT91_PMC_MCKR_MDIV_4:
		gd->arch.mck_rate_hz = freq / 4;
		break;
	default:
		break;
	}

	gd->arch.cpu_clk_rate_hz = freq;

	return 0;
}

void at91_plla_init(u32 pllar)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(pllar, &pmc->pllar);
	while (!(readl(&pmc->sr) & (AT91_PMC_LOCKA | AT91_PMC_MCKRDY)))
		;
}

void at91_mck_init(u32 mckr)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = readl(&pmc->mckr);
	tmp &= ~(AT91_PMC_MCKR_CSS_MASK  |
		 AT91_PMC_MCKR_PRES_MASK |
		 AT91_PMC_MCKR_MDIV_MASK |
		 AT91_PMC_MCKR_PLLADIV_2);
#ifdef CPU_HAS_H32MXDIV
	tmp &= ~AT91_PMC_MCKR_H32MXDIV;
#endif

	tmp |= mckr & (AT91_PMC_MCKR_CSS_MASK  |
		       AT91_PMC_MCKR_PRES_MASK |
		       AT91_PMC_MCKR_MDIV_MASK |
		       AT91_PMC_MCKR_PLLADIV_2);
#ifdef CPU_HAS_H32MXDIV
	tmp |= mckr & AT91_PMC_MCKR_H32MXDIV;
#endif

	writel(tmp, &pmc->mckr);

	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;
}

/*
 * For the Master Clock Controller Register(MCKR), while switching
 * to a lower clock source, we must switch the clock source first
 * instead of last. Otherwise, we could end up with too high frequency
 * on the internal bus and peripherals.
 */
void at91_mck_init_down(u32 mckr)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = readl(&pmc->mckr);
	tmp &= (~AT91_PMC_MCKR_CSS_MASK);
	tmp |= (mckr & AT91_PMC_MCKR_CSS_MASK);
	writel(tmp, &pmc->mckr);

	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;

#ifdef CPU_HAS_H32MXDIV
	tmp = readl(&pmc->mckr);
	tmp &= (~AT91_PMC_MCKR_H32MXDIV);
	tmp |= (mckr & AT91_PMC_MCKR_H32MXDIV);
	writel(tmp, &pmc->mckr);
#endif

	tmp = readl(&pmc->mckr);
	tmp &= (~AT91_PMC_MCKR_PLLADIV_MASK);
	tmp |= (mckr & AT91_PMC_MCKR_PLLADIV_MASK);
	writel(tmp, &pmc->mckr);

	tmp = readl(&pmc->mckr);
	tmp &= (~AT91_PMC_MCKR_MDIV_MASK);
	tmp |= (mckr & AT91_PMC_MCKR_MDIV_MASK);
	writel(tmp, &pmc->mckr);

	tmp = readl(&pmc->mckr);
	tmp &= (~AT91_PMC_MCKR_PRES_MASK);
	tmp |= (mckr & AT91_PMC_MCKR_PRES_MASK);
	writel(tmp, &pmc->mckr);
}

int at91_enable_periph_generated_clk(u32 id, u32 clk_source, u32 div)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 regval, status;
	u32 timeout = 1000;

	if (id > AT91_PMC_PCR_PID_MASK)
		return -EINVAL;

	if (div > 0xff)
		return -EINVAL;

	if (clk_source == GCK_CSS_UPLL_CLK) {
		if (at91_upll_clk_enable())
			return -ENODEV;
	}

	writel(id, &pmc->pcr);
	regval = readl(&pmc->pcr);
	regval &= ~AT91_PMC_PCR_GCKCSS;
	regval &= ~AT91_PMC_PCR_GCKDIV;

	switch (clk_source) {
	case GCK_CSS_SLOW_CLK:
		regval |= AT91_PMC_PCR_GCKCSS_SLOW_CLK;
		break;
	case GCK_CSS_MAIN_CLK:
		regval |= AT91_PMC_PCR_GCKCSS_MAIN_CLK;
		break;
	case GCK_CSS_PLLA_CLK:
		regval |= AT91_PMC_PCR_GCKCSS_PLLA_CLK;
		break;
	case GCK_CSS_UPLL_CLK:
		regval |= AT91_PMC_PCR_GCKCSS_UPLL_CLK;
		break;
	case GCK_CSS_MCK_CLK:
		regval |= AT91_PMC_PCR_GCKCSS_MCK_CLK;
		break;
	case GCK_CSS_AUDIO_CLK:
		regval |= AT91_PMC_PCR_GCKCSS_AUDIO_CLK;
		break;
	default:
		printf("Error GCK clock source selection!\n");
		return -EINVAL;
	}

	regval |= AT91_PMC_PCR_CMD_WRITE |
		  AT91_PMC_PCR_GCKDIV_(div) |
		  AT91_PMC_PCR_GCKEN;

	writel(regval, &pmc->pcr);

	do {
		udelay(1);
		status = readl(&pmc->sr);
	} while ((!!(--timeout)) && (!(status & AT91_PMC_GCKRDY)));

	if (!timeout)
		printf("Timeout waiting for GCK ready!\n");

	return 0;
}

u32 at91_get_periph_generated_clk(u32 id)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 regval, clk_source, div;
	u32 freq;

	if (id > AT91_PMC_PCR_PID_MASK)
		return 0;

	writel(id, &pmc->pcr);
	regval = readl(&pmc->pcr);

	clk_source = regval & AT91_PMC_PCR_GCKCSS;
	switch (clk_source) {
	case AT91_PMC_PCR_GCKCSS_SLOW_CLK:
		freq = CONFIG_SYS_AT91_SLOW_CLOCK;
		break;
	case AT91_PMC_PCR_GCKCSS_MAIN_CLK:
		freq = gd->arch.main_clk_rate_hz;
		break;
	case AT91_PMC_PCR_GCKCSS_PLLA_CLK:
		freq = gd->arch.plla_rate_hz;
		break;
	case AT91_PMC_PCR_GCKCSS_UPLL_CLK:
		freq = AT91_UTMI_PLL_CLK_FREQ;
		break;
	case AT91_PMC_PCR_GCKCSS_MCK_CLK:
		freq = gd->arch.mck_rate_hz;
		break;
	default:
		printf("Improper GCK clock source selection!\n");
		freq = 0;
		break;
	}

	div = ((regval & AT91_PMC_PCR_GCKDIV) >> AT91_PMC_PCR_GCKDIV_OFFSET);
	div += 1;

	return freq / div;
}
