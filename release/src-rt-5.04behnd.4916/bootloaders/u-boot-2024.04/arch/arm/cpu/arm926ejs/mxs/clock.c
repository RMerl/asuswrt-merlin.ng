// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX23/i.MX28 clock setup code
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <log.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>

/*
 * The PLL frequency is 480MHz and XTAL frequency is 24MHz
 *   iMX23: datasheet section 4.2
 *   iMX28: datasheet section 10.2
 */
#define	PLL_FREQ_KHZ	480000
#define	PLL_FREQ_COEF	18
#define	XTAL_FREQ_KHZ	24000

#define	PLL_FREQ_MHZ	(PLL_FREQ_KHZ / 1000)
#define	XTAL_FREQ_MHZ	(XTAL_FREQ_KHZ / 1000)

#if defined(CONFIG_MX23)
#define MXC_SSPCLK_MAX MXC_SSPCLK0
#elif defined(CONFIG_MX28)
#define MXC_SSPCLK_MAX MXC_SSPCLK3
#endif

static uint32_t mxs_get_pclk(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t clkctrl, clkseq, div;
	uint8_t clkfrac, frac;

	clkctrl = readl(&clkctrl_regs->hw_clkctrl_cpu);

	/* No support of fractional divider calculation */
	if (clkctrl &
		(CLKCTRL_CPU_DIV_XTAL_FRAC_EN | CLKCTRL_CPU_DIV_CPU_FRAC_EN)) {
		return 0;
	}

	clkseq = readl(&clkctrl_regs->hw_clkctrl_clkseq);

	/* XTAL Path */
	if (clkseq & CLKCTRL_CLKSEQ_BYPASS_CPU) {
		div = (clkctrl & CLKCTRL_CPU_DIV_XTAL_MASK) >>
			CLKCTRL_CPU_DIV_XTAL_OFFSET;
		return XTAL_FREQ_MHZ / div;
	}

	/* REF Path */
	clkfrac = readb(&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_CPU]);
	frac = clkfrac & CLKCTRL_FRAC_FRAC_MASK;
	div = clkctrl & CLKCTRL_CPU_DIV_CPU_MASK;
	return (PLL_FREQ_MHZ * PLL_FREQ_COEF / frac) / div;
}

static uint32_t mxs_get_hclk(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t div;
	uint32_t clkctrl;

	clkctrl = readl(&clkctrl_regs->hw_clkctrl_hbus);

	/* No support of fractional divider calculation */
	if (clkctrl & CLKCTRL_HBUS_DIV_FRAC_EN)
		return 0;

	div = clkctrl & CLKCTRL_HBUS_DIV_MASK;
	return mxs_get_pclk() / div;
}

static uint32_t mxs_get_emiclk(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	uint32_t clkctrl, clkseq, div;
	uint8_t clkfrac, frac;

	clkseq = readl(&clkctrl_regs->hw_clkctrl_clkseq);
	clkctrl = readl(&clkctrl_regs->hw_clkctrl_emi);

	/* XTAL Path */
	if (clkseq & CLKCTRL_CLKSEQ_BYPASS_EMI) {
		div = (clkctrl & CLKCTRL_EMI_DIV_XTAL_MASK) >>
			CLKCTRL_EMI_DIV_XTAL_OFFSET;
		return XTAL_FREQ_MHZ / div;
	}

	/* REF Path */
	clkfrac = readb(&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_EMI]);
	frac = clkfrac & CLKCTRL_FRAC_FRAC_MASK;
	div = clkctrl & CLKCTRL_EMI_DIV_EMI_MASK;
	return (PLL_FREQ_MHZ * PLL_FREQ_COEF / frac) / div;
}

static uint32_t mxs_get_gpmiclk(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
#if defined(CONFIG_MX23)
	uint8_t *reg =
		&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_CPU];
#elif defined(CONFIG_MX28)
	uint8_t *reg =
		&clkctrl_regs->hw_clkctrl_frac1[CLKCTRL_FRAC1_GPMI];
#endif
	uint32_t clkctrl, clkseq, div;
	uint8_t clkfrac, frac;

	clkseq = readl(&clkctrl_regs->hw_clkctrl_clkseq);
	clkctrl = readl(&clkctrl_regs->hw_clkctrl_gpmi);

	/* XTAL Path */
	if (clkseq & CLKCTRL_CLKSEQ_BYPASS_GPMI) {
		div = clkctrl & CLKCTRL_GPMI_DIV_MASK;
		return XTAL_FREQ_MHZ / div;
	}

	/* REF Path */
	clkfrac = readb(reg);
	frac = clkfrac & CLKCTRL_FRAC_FRAC_MASK;
	div = clkctrl & CLKCTRL_GPMI_DIV_MASK;
	return (PLL_FREQ_MHZ * PLL_FREQ_COEF / frac) / div;
}

/*
 * Set IO clock frequency, in kHz
 */
void mxs_set_ioclk(enum mxs_ioclock io, uint32_t freq)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t div;
	int io_reg;

	if (freq == 0)
		return;

	if ((io < MXC_IOCLK0) || (io > MXC_IOCLK1))
		return;

	div = (PLL_FREQ_KHZ * PLL_FREQ_COEF) / freq;

	if (div < 18)
		div = 18;

	if (div > 35)
		div = 35;

	io_reg = CLKCTRL_FRAC0_IO0 - io;	/* Register order is reversed */
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_set[io_reg]);
	writeb(CLKCTRL_FRAC_CLKGATE | (div & CLKCTRL_FRAC_FRAC_MASK),
		&clkctrl_regs->hw_clkctrl_frac0[io_reg]);
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_clr[io_reg]);
}

/*
 * Get IO clock, returns IO clock in kHz
 */
static uint32_t mxs_get_ioclk(enum mxs_ioclock io)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint8_t ret;
	int io_reg;

	if ((io < MXC_IOCLK0) || (io > MXC_IOCLK1))
		return 0;

	io_reg = CLKCTRL_FRAC0_IO0 - io;	/* Register order is reversed */

	ret = readb(&clkctrl_regs->hw_clkctrl_frac0[io_reg]) &
		CLKCTRL_FRAC_FRAC_MASK;

	return (PLL_FREQ_KHZ * PLL_FREQ_COEF) / ret;
}

/*
 * Configure SSP clock frequency, in kHz
 */
void mxs_set_sspclk(enum mxs_sspclock ssp, uint32_t freq, int xtal)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t clk, clkreg;

	if (ssp > MXC_SSPCLK_MAX)
		return;

	clkreg = (uint32_t)(&clkctrl_regs->hw_clkctrl_ssp0) +
			(ssp * sizeof(struct mxs_register_32));

	clrbits_le32(clkreg, CLKCTRL_SSP_CLKGATE);
	while (readl(clkreg) & CLKCTRL_SSP_CLKGATE)
		;

	if (xtal)
		clk = XTAL_FREQ_KHZ;
	else
		clk = mxs_get_ioclk(ssp >> 1);

	if (freq > clk)
		return;

	/* Calculate the divider and cap it if necessary */
	clk /= freq;
	if (clk > CLKCTRL_SSP_DIV_MASK)
		clk = CLKCTRL_SSP_DIV_MASK;

	clrsetbits_le32(clkreg, CLKCTRL_SSP_DIV_MASK, clk);
	while (readl(clkreg) & CLKCTRL_SSP_BUSY)
		;

	if (xtal)
		writel(CLKCTRL_CLKSEQ_BYPASS_SSP0 << ssp,
			&clkctrl_regs->hw_clkctrl_clkseq_set);
	else
		writel(CLKCTRL_CLKSEQ_BYPASS_SSP0 << ssp,
			&clkctrl_regs->hw_clkctrl_clkseq_clr);
}

/*
 * Return SSP frequency, in kHz
 */
static uint32_t mxs_get_sspclk(enum mxs_sspclock ssp)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t clkreg;
	uint32_t clk, tmp;

	if (ssp > MXC_SSPCLK_MAX)
		return 0;

	tmp = readl(&clkctrl_regs->hw_clkctrl_clkseq);
	if (tmp & (CLKCTRL_CLKSEQ_BYPASS_SSP0 << ssp))
		return XTAL_FREQ_KHZ;

	clkreg = (uint32_t)(&clkctrl_regs->hw_clkctrl_ssp0) +
			(ssp * sizeof(struct mxs_register_32));

	tmp = readl(clkreg) & CLKCTRL_SSP_DIV_MASK;

	if (tmp == 0)
		return 0;

	clk = mxs_get_ioclk(ssp >> 1);

	return clk / tmp;
}

/*
 * Set SSP/MMC bus frequency, in kHz)
 */
void mxs_set_ssp_busclock(unsigned int bus, uint32_t freq)
{
	struct mxs_ssp_regs *ssp_regs;
	const enum mxs_sspclock clk = mxs_ssp_clock_by_bus(bus);
	const uint32_t sspclk = mxs_get_sspclk(clk);
	uint32_t reg;
	uint32_t divide, rate, tgtclk;

	ssp_regs = mxs_ssp_regs_by_bus(bus);

	/*
	 * SSP bit rate = SSPCLK / (CLOCK_DIVIDE * (1 + CLOCK_RATE)),
	 * CLOCK_DIVIDE has to be an even value from 2 to 254, and
	 * CLOCK_RATE could be any integer from 0 to 255.
	 */
	for (divide = 2; divide < 254; divide += 2) {
		rate = sspclk / freq / divide;
		if (rate <= 256)
			break;
	}

	tgtclk = sspclk / divide / rate;
	while (tgtclk > freq) {
		rate++;
		tgtclk = sspclk / divide / rate;
	}
	if (rate > 256)
		rate = 256;

	/* Always set timeout the maximum */
	reg = SSP_TIMING_TIMEOUT_MASK |
		(divide << SSP_TIMING_CLOCK_DIVIDE_OFFSET) |
		((rate - 1) << SSP_TIMING_CLOCK_RATE_OFFSET);
	writel(reg, &ssp_regs->hw_ssp_timing);

	debug("SPI%d: Set freq rate to %d KHz (requested %d KHz)\n",
		bus, tgtclk, freq);
}

void mxs_set_lcdclk(uint32_t __maybe_unused lcd_base, uint32_t freq)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;
	uint32_t fp, x, k_rest, k_best, x_best, tk;
	int32_t k_best_l = 999, k_best_t = 0, x_best_l = 0xff, x_best_t = 0xff;

	if (freq == 0)
		return;

#if defined(CONFIG_MX23)
	writel(CLKCTRL_CLKSEQ_BYPASS_PIX, &clkctrl_regs->hw_clkctrl_clkseq_clr);
#elif defined(CONFIG_MX28)
	writel(CLKCTRL_CLKSEQ_BYPASS_DIS_LCDIF, &clkctrl_regs->hw_clkctrl_clkseq_clr);
#endif

	/*
	 *             /               18 \     1       1
	 * freq kHz = | 480000000 Hz * --  | * --- * ------
	 *             \                x /     k     1000
	 *
	 *      480000000 Hz   18
	 *      ------------ * --
	 *        freq kHz      x
	 * k = -------------------
	 *             1000
	 */

	fp = ((PLL_FREQ_KHZ * 1000) / freq) * 18;

	for (x = 18; x <= 35; x++) {
		tk = fp / x;
		if ((tk / 1000 == 0) || (tk / 1000 > 255))
			continue;

		k_rest = tk % 1000;

		if (k_rest < (k_best_l % 1000)) {
			k_best_l = tk;
			x_best_l = x;
		}

		if (k_rest > (k_best_t % 1000)) {
			k_best_t = tk;
			x_best_t = x;
		}
	}

	if (1000 - (k_best_t % 1000) > (k_best_l % 1000)) {
		k_best = k_best_l;
		x_best = x_best_l;
	} else {
		k_best = k_best_t;
		x_best = x_best_t;
	}

	k_best /= 1000;

#if defined(CONFIG_MX23)
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_set[CLKCTRL_FRAC0_PIX]);
	writeb(CLKCTRL_FRAC_CLKGATE | (x_best & CLKCTRL_FRAC_FRAC_MASK),
		&clkctrl_regs->hw_clkctrl_frac0[CLKCTRL_FRAC0_PIX]);
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac0_clr[CLKCTRL_FRAC0_PIX]);

	writel(CLKCTRL_PIX_CLKGATE,
		&clkctrl_regs->hw_clkctrl_pix_set);
	clrsetbits_le32(&clkctrl_regs->hw_clkctrl_pix,
			CLKCTRL_PIX_DIV_MASK | CLKCTRL_PIX_CLKGATE,
			k_best << CLKCTRL_PIX_DIV_OFFSET);

	while (readl(&clkctrl_regs->hw_clkctrl_pix) & CLKCTRL_PIX_BUSY)
		;
#elif defined(CONFIG_MX28)
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac1_set[CLKCTRL_FRAC1_PIX]);
	writeb(CLKCTRL_FRAC_CLKGATE | (x_best & CLKCTRL_FRAC_FRAC_MASK),
		&clkctrl_regs->hw_clkctrl_frac1[CLKCTRL_FRAC1_PIX]);
	writeb(CLKCTRL_FRAC_CLKGATE,
		&clkctrl_regs->hw_clkctrl_frac1_clr[CLKCTRL_FRAC1_PIX]);

	writel(CLKCTRL_DIS_LCDIF_CLKGATE,
		&clkctrl_regs->hw_clkctrl_lcdif_set);
	clrsetbits_le32(&clkctrl_regs->hw_clkctrl_lcdif,
			CLKCTRL_DIS_LCDIF_DIV_MASK | CLKCTRL_DIS_LCDIF_CLKGATE,
			k_best << CLKCTRL_DIS_LCDIF_DIV_OFFSET);

	while (readl(&clkctrl_regs->hw_clkctrl_lcdif) & CLKCTRL_DIS_LCDIF_BUSY)
		;
#endif
}

uint32_t mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return mxs_get_pclk() * 1000000;
	case MXC_GPMI_CLK:
		return mxs_get_gpmiclk() * 1000000;
	case MXC_AHB_CLK:
	case MXC_IPG_CLK:
		return mxs_get_hclk() * 1000000;
	case MXC_EMI_CLK:
		return mxs_get_emiclk();
	case MXC_IO0_CLK:
		return mxs_get_ioclk(MXC_IOCLK0);
	case MXC_IO1_CLK:
		return mxs_get_ioclk(MXC_IOCLK1);
	case MXC_XTAL_CLK:
		return XTAL_FREQ_KHZ * 1000;
	case MXC_SSP0_CLK:
		return mxs_get_sspclk(MXC_SSPCLK0);
#ifdef CONFIG_MX28
	case MXC_SSP1_CLK:
		return mxs_get_sspclk(MXC_SSPCLK1);
	case MXC_SSP2_CLK:
		return mxs_get_sspclk(MXC_SSPCLK2);
	case MXC_SSP3_CLK:
		return mxs_get_sspclk(MXC_SSPCLK3);
#endif
	}

	return 0;
}
