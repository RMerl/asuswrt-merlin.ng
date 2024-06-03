// SPDX-License-Identifier: GPL-2.0+
/*
 * [origin: Linux kernel linux/arch/arm/mach-at91/clock.c]
 *
 * Copyright (C) 2005 David Brownell
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>

#if !defined(CONFIG_AT91FAMILY)
# error You need to define CONFIG_AT91FAMILY in your board config!
#endif

#define EN_PLLB_TIMEOUT	500

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
	case AT91_PMC_MCKR_CSS_PLLB:
		return gd->arch.pllb_rate_hz;
	}

	return 0;
}

#ifdef CONFIG_USB_ATMEL
static unsigned at91_pll_calc(unsigned main_freq, unsigned out_freq)
{
	unsigned i, div = 0, mul = 0, diff = 1 << 30;
	unsigned ret = (out_freq > 155000000) ? 0xbe00 : 0x3e00;

	/* PLL output max 240 MHz (or 180 MHz per errata) */
	if (out_freq > 240000000)
		goto fail;

	for (i = 1; i < 256; i++) {
		int diff1;
		unsigned input, mul1;

		/*
		 * PLL input between 1MHz and 32MHz per spec, but lower
		 * frequences seem necessary in some cases so allow 100K.
		 * Warning: some newer products need 2MHz min.
		 */
		input = main_freq / i;
#if defined(CONFIG_AT91SAM9G20)
		if (input < 2000000)
			continue;
#endif
		if (input < 100000)
			continue;
		if (input > 32000000)
			continue;

		mul1 = out_freq / input;
#if defined(CONFIG_AT91SAM9G20)
		if (mul > 63)
			continue;
#endif
		if (mul1 > 2048)
			continue;
		if (mul1 < 2)
			goto fail;

		diff1 = out_freq - input * mul1;
		if (diff1 < 0)
			diff1 = -diff1;
		if (diff > diff1) {
			diff = diff1;
			div = i;
			mul = mul1;
			if (diff == 0)
				break;
		}
	}
	if (i == 256 && diff > (out_freq >> 5))
		goto fail;
	return ret | ((mul - 1) << 16) | div;
fail:
	return 0;
}
#endif

static u32 at91_pll_rate(u32 freq, u32 reg)
{
	unsigned mul, div;

	div = reg & 0xff;
	mul = (reg >> 16) & 0x7ff;
	if (div && mul) {
		freq /= div;
		freq *= mul + 1;
	} else
		freq = 0;

	return freq;
}

int at91_clock_init(unsigned long main_clock)
{
	unsigned freq, mckr;
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;
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

#ifdef CONFIG_USB_ATMEL
	/*
	 * USB clock init:  choose 48 MHz PLLB value,
	 * disable 48MHz clock during usb peripheral suspend.
	 *
	 * REVISIT:  assumes MCK doesn't derive from PLLB!
	 */
	gd->arch.at91_pllb_usb_init = at91_pll_calc(main_clock, 48000000 * 2) |
			     AT91_PMC_PLLBR_USBDIV_2;
	gd->arch.pllb_rate_hz = at91_pll_rate(main_clock,
					      gd->arch.at91_pllb_usb_init);
#endif

	/*
	 * MCK and CPU derive from one of those primary clocks.
	 * For now, assume this parentage won't change.
	 */
	mckr = readl(&pmc->mckr);
#if defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45) \
		|| defined(CONFIG_AT91SAM9N12) || defined(CONFIG_AT91SAM9X5)
	/* plla divisor by 2 */
	gd->arch.plla_rate_hz /= (1 << ((mckr & 1 << 12) >> 12));
#endif
	gd->arch.mck_rate_hz = at91_css_to_rate(mckr & AT91_PMC_MCKR_CSS_MASK);
	freq = gd->arch.mck_rate_hz;

#if defined(CONFIG_AT91SAM9X5)
	/* different in prescale on at91sam9x5 */
	freq /= (1 << ((mckr & AT91_PMC_MCKR_PRES_MASK) >> 4));
#else
	freq /= (1 << ((mckr & AT91_PMC_MCKR_PRES_MASK) >> 2));	/* prescale */
#endif

#if defined(CONFIG_AT91SAM9G20)
	/* mdiv ; (x >> 7) = ((x >> 8) * 2) */
	gd->arch.mck_rate_hz = (mckr & AT91_PMC_MCKR_MDIV_MASK) ?
		freq / ((mckr & AT91_PMC_MCKR_MDIV_MASK) >> 7) : freq;
	if (mckr & AT91_PMC_MCKR_MDIV_MASK)
		freq /= 2;			/* processor clock division */
#elif defined(CONFIG_AT91SAM9G45) || defined(CONFIG_AT91SAM9M10G45) \
		|| defined(CONFIG_AT91SAM9N12) || defined(CONFIG_AT91SAM9X5)
	/* mdiv <==> divisor
	 *  0   <==>   1
	 *  1   <==>   2
	 *  2   <==>   4
	 *  3   <==>   3
	 */
	gd->arch.mck_rate_hz = (mckr & AT91_PMC_MCKR_MDIV_MASK) ==
		(AT91_PMC_MCKR_MDIV_2 | AT91_PMC_MCKR_MDIV_4)
		? freq / 3
		: freq / (1 << ((mckr & AT91_PMC_MCKR_MDIV_MASK) >> 8));
#else
	gd->arch.mck_rate_hz = freq /
			(1 << ((mckr & AT91_PMC_MCKR_MDIV_MASK) >> 8));
#endif
	gd->arch.cpu_clk_rate_hz = freq;

	return 0;
}

#if !defined(AT91_PLL_LOCK_TIMEOUT)
#define AT91_PLL_LOCK_TIMEOUT	1000000
#endif

void at91_plla_init(u32 pllar)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(pllar, &pmc->pllar);
	while (!(readl(&pmc->sr) & AT91_PMC_LOCKA))
		;
}
void at91_pllb_init(u32 pllbr)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(pllbr, &pmc->pllbr);
	while (!(readl(&pmc->sr) & AT91_PMC_LOCKB))
		;
}

void at91_mck_init(u32 mckr)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = readl(&pmc->mckr);
	tmp &= ~AT91_PMC_MCKR_PRES_MASK;
	tmp |= mckr & AT91_PMC_MCKR_PRES_MASK;
	writel(tmp, &pmc->mckr);
	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;

	tmp = readl(&pmc->mckr);
	tmp &= ~AT91_PMC_MCKR_MDIV_MASK;
	tmp |= mckr & AT91_PMC_MCKR_MDIV_MASK;
	writel(tmp, &pmc->mckr);
	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;

	tmp = readl(&pmc->mckr);
	tmp &= ~AT91_PMC_MCKR_PLLADIV_MASK;
	tmp |= mckr & AT91_PMC_MCKR_PLLADIV_MASK;
	writel(tmp, &pmc->mckr);
	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;

	tmp = readl(&pmc->mckr);
	tmp &= ~AT91_PMC_MCKR_CSS_MASK;
	tmp |= mckr & AT91_PMC_MCKR_CSS_MASK;
	writel(tmp, &pmc->mckr);
	while (!(readl(&pmc->sr) & AT91_PMC_MCKRDY))
		;
}

int at91_pllb_clk_enable(u32 pllbr)
{
	struct at91_pmc *pmc = (at91_pmc_t *)ATMEL_BASE_PMC;
	ulong start_time, tmp_time;

	start_time = get_timer(0);
	writel(pllbr, &pmc->pllbr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKB) != AT91_PMC_LOCKB) {
		tmp_time = get_timer(0);
		if ((tmp_time - start_time) > EN_PLLB_TIMEOUT) {
			printf("ERROR: failed to enable PLLB\n");
			return -1;
		}
	}

	return 0;
}

int at91_pllb_clk_disable(void)
{
	struct at91_pmc *pmc = (at91_pmc_t *)ATMEL_BASE_PMC;
	ulong start_time, tmp_time;

	start_time = get_timer(0);
	writel(0, &pmc->pllbr);
	while ((readl(&pmc->sr) & AT91_PMC_LOCKB) != 0) {
		tmp_time = get_timer(0);
		if ((tmp_time - start_time) > EN_PLLB_TIMEOUT) {
			printf("ERROR: failed to disable PLLB\n");
			return -1;
		}
	}

	return 0;
}
