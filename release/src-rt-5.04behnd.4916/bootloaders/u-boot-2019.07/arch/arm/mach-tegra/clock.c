// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra SoC common clock control functions */

#include <common.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/timer.h>

/*
 * This is our record of the current clock rate of each clock. We don't
 * fill all of these in since we are only really interested in clocks which
 * we use as parents.
 */
static unsigned pll_rate[CLOCK_ID_COUNT];

/*
 * The oscillator frequency is fixed to one of four set values. Based on this
 * the other clocks are set up appropriately.
 */
static unsigned osc_freq[CLOCK_OSC_FREQ_COUNT] = {
	13000000,
	19200000,
	12000000,
	26000000,
	38400000,
	48000000,
};

/* return 1 if a peripheral ID is in range */
#define clock_type_id_isvalid(id) ((id) >= 0 && \
		(id) < CLOCK_TYPE_COUNT)

char pllp_valid = 1;	/* PLLP is set up correctly */

/* return 1 if a periphc_internal_id is in range */
#define periphc_internal_id_isvalid(id) ((id) >= 0 && \
		(id) < PERIPHC_COUNT)

/* number of clock outputs of a PLL */
static const u8 pll_num_clkouts[] = {
	1,	/* PLLC */
	1,	/* PLLM */
	4,	/* PLLP */
	1,	/* PLLA */
	0,	/* PLLU */
	0,	/* PLLD */
};

int clock_get_osc_bypass(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	return (reg & OSC_XOBP_MASK) >> OSC_XOBP_SHIFT;
}

/* Returns a pointer to the registers of the given pll */
static struct clk_pll *get_pll(enum clock_id clkid)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;

	assert(clock_id_is_pll(clkid));
	if (clkid >= (enum clock_id)TEGRA_CLK_PLLS) {
		debug("%s: Invalid PLL %d\n", __func__, clkid);
		return NULL;
	}
	return &clkrst->crc_pll[clkid];
}

__weak struct clk_pll_simple *clock_get_simple_pll(enum clock_id clkid)
{
	return NULL;
}

int clock_ll_read_pll(enum clock_id clkid, u32 *divm, u32 *divn,
		u32 *divp, u32 *cpcon, u32 *lfcon)
{
	struct clk_pll *pll = get_pll(clkid);
	struct clk_pll_info *pllinfo = &tegra_pll_info_table[clkid];
	u32 data;

	assert(clkid != CLOCK_ID_USB);

	/* Safety check, adds to code size but is small */
	if (!clock_id_is_pll(clkid) || clkid == CLOCK_ID_USB)
		return -1;
	data = readl(&pll->pll_base);
	*divm = (data >> pllinfo->m_shift) & pllinfo->m_mask;
	*divn = (data >> pllinfo->n_shift) & pllinfo->n_mask;
	*divp = (data >> pllinfo->p_shift) & pllinfo->p_mask;
	data = readl(&pll->pll_misc);
	/* NOTE: On T210, cpcon/lfcon no longer exist, moved to KCP/KVCO */
	*cpcon = (data >> pllinfo->kcp_shift) & pllinfo->kcp_mask;
	*lfcon = (data >> pllinfo->kvco_shift) & pllinfo->kvco_mask;

	return 0;
}

unsigned long clock_start_pll(enum clock_id clkid, u32 divm, u32 divn,
		u32 divp, u32 cpcon, u32 lfcon)
{
	struct clk_pll *pll = NULL;
	struct clk_pll_info *pllinfo = &tegra_pll_info_table[clkid];
	struct clk_pll_simple *simple_pll = NULL;
	u32 misc_data, data;

	if (clkid < (enum clock_id)TEGRA_CLK_PLLS) {
		pll = get_pll(clkid);
	} else {
		simple_pll = clock_get_simple_pll(clkid);
		if (!simple_pll) {
			debug("%s: Uknown simple PLL %d\n", __func__, clkid);
			return 0;
		}
	}

	/*
	 * pllinfo has the m/n/p and kcp/kvco mask and shift
	 * values for all of the PLLs used in U-Boot, with any
	 * SoC differences accounted for.
	 *
	 * Preserve EN_LOCKDET, etc.
	 */
	if (pll)
		misc_data = readl(&pll->pll_misc);
	else
		misc_data = readl(&simple_pll->pll_misc);
	misc_data &= ~(pllinfo->kcp_mask << pllinfo->kcp_shift);
	misc_data |= cpcon << pllinfo->kcp_shift;
	misc_data &= ~(pllinfo->kvco_mask << pllinfo->kvco_shift);
	misc_data |= lfcon << pllinfo->kvco_shift;

	data = (divm << pllinfo->m_shift) | (divn << pllinfo->n_shift);
	data |= divp << pllinfo->p_shift;
	data |= (1 << PLL_ENABLE_SHIFT);	/* BYPASS s/b 0 already */

	if (pll) {
		writel(misc_data, &pll->pll_misc);
		writel(data, &pll->pll_base);
	} else {
		writel(misc_data, &simple_pll->pll_misc);
		writel(data, &simple_pll->pll_base);
	}

	/* calculate the stable time */
	return timer_get_us() + CLOCK_PLL_STABLE_DELAY_US;
}

void clock_ll_set_source_divisor(enum periph_id periph_id, unsigned source,
			unsigned divisor)
{
	u32 *reg = get_periph_source_reg(periph_id);
	u32 value;

	value = readl(reg);

	value &= ~OUT_CLK_SOURCE_31_30_MASK;
	value |= source << OUT_CLK_SOURCE_31_30_SHIFT;

	value &= ~OUT_CLK_DIVISOR_MASK;
	value |= divisor << OUT_CLK_DIVISOR_SHIFT;

	writel(value, reg);
}

int clock_ll_set_source_bits(enum periph_id periph_id, int mux_bits,
			     unsigned source)
{
	u32 *reg = get_periph_source_reg(periph_id);

	switch (mux_bits) {
	case MASK_BITS_31_30:
		clrsetbits_le32(reg, OUT_CLK_SOURCE_31_30_MASK,
				source << OUT_CLK_SOURCE_31_30_SHIFT);
		break;

	case MASK_BITS_31_29:
		clrsetbits_le32(reg, OUT_CLK_SOURCE_31_29_MASK,
				source << OUT_CLK_SOURCE_31_29_SHIFT);
		break;

	case MASK_BITS_31_28:
		clrsetbits_le32(reg, OUT_CLK_SOURCE_31_28_MASK,
				source << OUT_CLK_SOURCE_31_28_SHIFT);
		break;

	default:
		return -1;
	}

	return 0;
}

static int clock_ll_get_source_bits(enum periph_id periph_id, int mux_bits)
{
	u32 *reg = get_periph_source_reg(periph_id);
	u32 val = readl(reg);

	switch (mux_bits) {
	case MASK_BITS_31_30:
		val >>= OUT_CLK_SOURCE_31_30_SHIFT;
		val &= OUT_CLK_SOURCE_31_30_MASK;
		return val;
	case MASK_BITS_31_29:
		val >>= OUT_CLK_SOURCE_31_29_SHIFT;
		val &= OUT_CLK_SOURCE_31_29_MASK;
		return val;
	case MASK_BITS_31_28:
		val >>= OUT_CLK_SOURCE_31_28_SHIFT;
		val &= OUT_CLK_SOURCE_31_28_MASK;
		return val;
	default:
		return -1;
	}
}

void clock_ll_set_source(enum periph_id periph_id, unsigned source)
{
	clock_ll_set_source_bits(periph_id, MASK_BITS_31_30, source);
}

/**
 * Given the parent's rate and the required rate for the children, this works
 * out the peripheral clock divider to use, in 7.1 binary format.
 *
 * @param divider_bits	number of divider bits (8 or 16)
 * @param parent_rate	clock rate of parent clock in Hz
 * @param rate		required clock rate for this clock
 * @return divider which should be used
 */
static int clk_get_divider(unsigned divider_bits, unsigned long parent_rate,
			   unsigned long rate)
{
	u64 divider = parent_rate * 2;
	unsigned max_divider = 1 << divider_bits;

	divider += rate - 1;
	do_div(divider, rate);

	if ((s64)divider - 2 < 0)
		return 0;

	if ((s64)divider - 2 >= max_divider)
		return -1;

	return divider - 2;
}

int clock_set_pllout(enum clock_id clkid, enum pll_out_id pllout, unsigned rate)
{
	struct clk_pll *pll = get_pll(clkid);
	int data = 0, div = 0, offset = 0;

	if (!clock_id_is_pll(clkid))
		return -1;

	if (pllout + 1 > pll_num_clkouts[clkid])
		return -1;

	div = clk_get_divider(8, pll_rate[clkid], rate);

	if (div < 0)
		return -1;

	/* out2 and out4 are in the high part of the register */
	if (pllout == PLL_OUT2 || pllout == PLL_OUT4)
		offset = 16;

	data = (div << PLL_OUT_RATIO_SHIFT) |
			PLL_OUT_OVRRIDE | PLL_OUT_CLKEN | PLL_OUT_RSTN;
	clrsetbits_le32(&pll->pll_out[pllout >> 1],
			PLL_OUT_RATIO_MASK << offset, data << offset);

	return 0;
}

/**
 * Given the parent's rate and the divider in 7.1 format, this works out the
 * resulting peripheral clock rate.
 *
 * @param parent_rate	clock rate of parent clock in Hz
 * @param divider which should be used in 7.1 format
 * @return effective clock rate of peripheral
 */
static unsigned long get_rate_from_divider(unsigned long parent_rate,
					   int divider)
{
	u64 rate;

	rate = (u64)parent_rate * 2;
	do_div(rate, divider + 2);
	return rate;
}

unsigned long clock_get_periph_rate(enum periph_id periph_id,
		enum clock_id parent)
{
	u32 *reg = get_periph_source_reg(periph_id);
	unsigned parent_rate = pll_rate[parent];
	int div = (readl(reg) & OUT_CLK_DIVISOR_MASK) >> OUT_CLK_DIVISOR_SHIFT;

	switch (periph_id) {
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
	case PERIPH_ID_UART4:
	case PERIPH_ID_UART5:
#ifdef CONFIG_TEGRA20
		/* There's no divider for these clocks in this SoC. */
		return parent_rate;
#else
		/*
		 * This undoes the +2 in get_rate_from_divider() which I
		 * believe is incorrect. Ideally we would fix
		 * get_rate_from_divider(), but... Removing the +2 from
		 * get_rate_from_divider() would probably require remove the -2
		 * from the tail of clk_get_divider() since I believe that's
		 * only there to invert get_rate_from_divider()'s +2. Observe
		 * how find_best_divider() uses those two functions together.
		 * However, doing so breaks other stuff, such as Seaboard's
		 * display, likely due to clock_set_pllout()'s call to
		 * clk_get_divider(). Attempting to fix that by making
		 * clock_set_pllout() subtract 2 from clk_get_divider()'s
		 * return value doesn't help. In summary this clock driver is
		 * quite broken but I'm afraid I have no idea how to fix it
		 * without completely replacing it.
		 *
		 * Be careful to avoid a divide by zero error.
		 */
		if (div >= 1)
			div -= 2;
		break;
#endif
	default:
		break;
	}

	return get_rate_from_divider(parent_rate, div);
}

/**
 * Find the best available 7.1 format divisor given a parent clock rate and
 * required child clock rate. This function assumes that a second-stage
 * divisor is available which can divide by powers of 2 from 1 to 256.
 *
 * @param divider_bits	number of divider bits (8 or 16)
 * @param parent_rate	clock rate of parent clock in Hz
 * @param rate		required clock rate for this clock
 * @param extra_div	value for the second-stage divisor (not set if this
 *			function returns -1.
 * @return divider which should be used, or -1 if nothing is valid
 *
 */
static int find_best_divider(unsigned divider_bits, unsigned long parent_rate,
				unsigned long rate, int *extra_div)
{
	int shift;
	int best_divider = -1;
	int best_error = rate;

	/* try dividers from 1 to 256 and find closest match */
	for (shift = 0; shift <= 8 && best_error > 0; shift++) {
		unsigned divided_parent = parent_rate >> shift;
		int divider = clk_get_divider(divider_bits, divided_parent,
						rate);
		unsigned effective_rate = get_rate_from_divider(divided_parent,
						divider);
		int error = rate - effective_rate;

		/* Given a valid divider, look for the lowest error */
		if (divider != -1 && error < best_error) {
			best_error = error;
			*extra_div = 1 << shift;
			best_divider = divider;
		}
	}

	/* return what we found - *extra_div will already be set */
	return best_divider;
}

/**
 * Adjust peripheral PLL to use the given divider and source.
 *
 * @param periph_id	peripheral to adjust
 * @param source	Source number (0-3 or 0-7)
 * @param mux_bits	Number of mux bits (2 or 4)
 * @param divider	Required divider in 7.1 or 15.1 format
 * @return 0 if ok, -1 on error (requesting a parent clock which is not valid
 *		for this peripheral)
 */
static int adjust_periph_pll(enum periph_id periph_id, int source,
				int mux_bits, unsigned divider)
{
	u32 *reg = get_periph_source_reg(periph_id);

	clrsetbits_le32(reg, OUT_CLK_DIVISOR_MASK,
			divider << OUT_CLK_DIVISOR_SHIFT);
	udelay(1);

	/* work out the source clock and set it */
	if (source < 0)
		return -1;

	clock_ll_set_source_bits(periph_id, mux_bits, source);

	udelay(2);
	return 0;
}

enum clock_id clock_get_periph_parent(enum periph_id periph_id)
{
	int err, mux_bits, divider_bits, type;
	int source;

	err = get_periph_clock_info(periph_id, &mux_bits, &divider_bits, &type);
	if (err)
		return CLOCK_ID_NONE;

	source = clock_ll_get_source_bits(periph_id, mux_bits);

	return get_periph_clock_id(periph_id, source);
}

unsigned clock_adjust_periph_pll_div(enum periph_id periph_id,
		enum clock_id parent, unsigned rate, int *extra_div)
{
	unsigned effective_rate;
	int mux_bits, divider_bits, source;
	int divider;
	int xdiv = 0;

	/* work out the source clock and set it */
	source = get_periph_clock_source(periph_id, parent, &mux_bits,
					 &divider_bits);

	divider = find_best_divider(divider_bits, pll_rate[parent],
				    rate, &xdiv);
	if (extra_div)
		*extra_div = xdiv;

	assert(divider >= 0);
	if (adjust_periph_pll(periph_id, source, mux_bits, divider))
		return -1U;
	debug("periph %d, rate=%d, reg=%p = %x\n", periph_id, rate,
		get_periph_source_reg(periph_id),
		readl(get_periph_source_reg(periph_id)));

	/* Check what we ended up with. This shouldn't matter though */
	effective_rate = clock_get_periph_rate(periph_id, parent);
	if (extra_div)
		effective_rate /= *extra_div;
	if (rate != effective_rate)
		debug("Requested clock rate %u not honored (got %u)\n",
			rate, effective_rate);
	return effective_rate;
}

unsigned clock_start_periph_pll(enum periph_id periph_id,
		enum clock_id parent, unsigned rate)
{
	unsigned effective_rate;

	reset_set_enable(periph_id, 1);
	clock_enable(periph_id);
	udelay(2);

	effective_rate = clock_adjust_periph_pll_div(periph_id, parent, rate,
						 NULL);

	reset_set_enable(periph_id, 0);
	return effective_rate;
}

void clock_enable(enum periph_id clkid)
{
	clock_set_enable(clkid, 1);
}

void clock_disable(enum periph_id clkid)
{
	clock_set_enable(clkid, 0);
}

void reset_periph(enum periph_id periph_id, int us_delay)
{
	/* Put peripheral into reset */
	reset_set_enable(periph_id, 1);
	udelay(us_delay);

	/* Remove reset */
	reset_set_enable(periph_id, 0);

	udelay(us_delay);
}

void reset_cmplx_set_enable(int cpu, int which, int reset)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 mask;

	/* Form the mask, which depends on the cpu chosen (2 or 4) */
	assert(cpu >= 0 && cpu < MAX_NUM_CPU);
	mask = which << cpu;

	/* either enable or disable those reset for that CPU */
	if (reset)
		writel(mask, &clkrst->crc_cpu_cmplx_set);
	else
		writel(mask, &clkrst->crc_cpu_cmplx_clr);
}

unsigned int __weak clk_m_get_rate(unsigned int parent_rate)
{
	return parent_rate;
}

unsigned clock_get_rate(enum clock_id clkid)
{
	struct clk_pll *pll;
	u32 base, divm;
	u64 parent_rate, rate;
	struct clk_pll_info *pllinfo = &tegra_pll_info_table[clkid];

	parent_rate = osc_freq[clock_get_osc_freq()];
	if (clkid == CLOCK_ID_OSC)
		return parent_rate;

	if (clkid == CLOCK_ID_CLK_M)
		return clk_m_get_rate(parent_rate);

	pll = get_pll(clkid);
	if (!pll)
		return 0;
	base = readl(&pll->pll_base);

	rate = parent_rate * ((base >> pllinfo->n_shift) & pllinfo->n_mask);
	divm = (base >> pllinfo->m_shift) & pllinfo->m_mask;
	/*
	 * PLLU uses p_mask/p_shift for VCO on all but T210,
	 * T210 uses normal DIVP. Handled in pllinfo table.
	 */
#ifdef CONFIG_TEGRA210
	/*
	 * PLLP's primary output (pllP_out0) on T210 is the VCO, and divp is
	 * not applied. pllP_out2 does have divp applied. All other pllP_outN
	 * are divided down from pllP_out0. We only support pllP_out0 in
	 * U-Boot at the time of writing this comment.
	 */
	if (clkid != CLOCK_ID_PERIPH)
#endif
		divm <<= (base >> pllinfo->p_shift) & pllinfo->p_mask;
	do_div(rate, divm);
	return rate;
}

/**
 * Set the output frequency you want for each PLL clock.
 * PLL output frequencies are programmed by setting their N, M and P values.
 * The governing equations are:
 *     VCO = (Fi / m) * n, Fo = VCO / (2^p)
 *     where Fo is the output frequency from the PLL.
 * Example: Set the output frequency to 216Mhz(Fo) with 12Mhz OSC(Fi)
 *     216Mhz = ((12Mhz / m) * n) / (2^p) so n=432,m=12,p=1
 * Please see Tegra TRM section 5.3 to get the detail for PLL Programming
 *
 * @param n PLL feedback divider(DIVN)
 * @param m PLL input divider(DIVN)
 * @param p post divider(DIVP)
 * @param cpcon base PLL charge pump(CPCON)
 * @return 0 if ok, -1 on error (the requested PLL is incorrect and cannot
 *		be overridden), 1 if PLL is already correct
 */
int clock_set_rate(enum clock_id clkid, u32 n, u32 m, u32 p, u32 cpcon)
{
	u32 base_reg, misc_reg;
	struct clk_pll *pll;
	struct clk_pll_info *pllinfo = &tegra_pll_info_table[clkid];

	pll = get_pll(clkid);

	base_reg = readl(&pll->pll_base);

	/* Set BYPASS, m, n and p to PLL_BASE */
	base_reg &= ~(pllinfo->m_mask << pllinfo->m_shift);
	base_reg |= m << pllinfo->m_shift;

	base_reg &= ~(pllinfo->n_mask << pllinfo->n_shift);
	base_reg |= n << pllinfo->n_shift;

	base_reg &= ~(pllinfo->p_mask << pllinfo->p_shift);
	base_reg |= p << pllinfo->p_shift;

	if (clkid == CLOCK_ID_PERIPH) {
		/*
		 * If the PLL is already set up, check that it is correct
		 * and record this info for clock_verify() to check.
		 */
		if (base_reg & PLL_BASE_OVRRIDE_MASK) {
			base_reg |= PLL_ENABLE_MASK;
			if (base_reg != readl(&pll->pll_base))
				pllp_valid = 0;
			return pllp_valid ? 1 : -1;
		}
		base_reg |= PLL_BASE_OVRRIDE_MASK;
	}

	base_reg |= PLL_BYPASS_MASK;
	writel(base_reg, &pll->pll_base);

	/* Set cpcon (KCP) to PLL_MISC */
	misc_reg = readl(&pll->pll_misc);
	misc_reg &= ~(pllinfo->kcp_mask << pllinfo->kcp_shift);
	misc_reg |= cpcon << pllinfo->kcp_shift;
	writel(misc_reg, &pll->pll_misc);

	/* Enable PLL */
	base_reg |= PLL_ENABLE_MASK;
	writel(base_reg, &pll->pll_base);

	/* Disable BYPASS */
	base_reg &= ~PLL_BYPASS_MASK;
	writel(base_reg, &pll->pll_base);

	return 0;
}

void clock_ll_start_uart(enum periph_id periph_id)
{
	/* Assert UART reset and enable clock */
	reset_set_enable(periph_id, 1);
	clock_enable(periph_id);
	clock_ll_set_source(periph_id, 0); /* UARTx_CLK_SRC = 00, PLLP_OUT0 */

	/* wait for 2us */
	udelay(2);

	/* De-assert reset to UART */
	reset_set_enable(periph_id, 0);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
int clock_decode_periph_id(struct udevice *dev)
{
	enum periph_id id;
	u32 cell[2];
	int err;

	err = dev_read_u32_array(dev, "clocks", cell, ARRAY_SIZE(cell));
	if (err)
		return -1;
	id = clk_id_to_periph_id(cell[1]);
	assert(clock_periph_id_isvalid(id));
	return id;
}
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

int clock_verify(void)
{
	struct clk_pll *pll = get_pll(CLOCK_ID_PERIPH);
	u32 reg = readl(&pll->pll_base);

	if (!pllp_valid) {
		printf("Warning: PLLP %x is not correct\n", reg);
		return -1;
	}
	debug("PLLP %x is correct\n", reg);
	return 0;
}

void clock_init(void)
{
	int i;

	pll_rate[CLOCK_ID_CGENERAL] = clock_get_rate(CLOCK_ID_CGENERAL);
	pll_rate[CLOCK_ID_MEMORY] = clock_get_rate(CLOCK_ID_MEMORY);
	pll_rate[CLOCK_ID_PERIPH] = clock_get_rate(CLOCK_ID_PERIPH);
	pll_rate[CLOCK_ID_USB] = clock_get_rate(CLOCK_ID_USB);
	pll_rate[CLOCK_ID_DISPLAY] = clock_get_rate(CLOCK_ID_DISPLAY);
	pll_rate[CLOCK_ID_XCPU] = clock_get_rate(CLOCK_ID_XCPU);
	pll_rate[CLOCK_ID_SFROM32KHZ] = 32768;
	pll_rate[CLOCK_ID_OSC] = clock_get_rate(CLOCK_ID_OSC);
	pll_rate[CLOCK_ID_CLK_M] = clock_get_rate(CLOCK_ID_CLK_M);

	debug("Osc = %d\n", pll_rate[CLOCK_ID_OSC]);
	debug("CLKM = %d\n", pll_rate[CLOCK_ID_CLK_M]);
	debug("PLLC = %d\n", pll_rate[CLOCK_ID_CGENERAL]);
	debug("PLLM = %d\n", pll_rate[CLOCK_ID_MEMORY]);
	debug("PLLP = %d\n", pll_rate[CLOCK_ID_PERIPH]);
	debug("PLLU = %d\n", pll_rate[CLOCK_ID_USB]);
	debug("PLLD = %d\n", pll_rate[CLOCK_ID_DISPLAY]);
	debug("PLLX = %d\n", pll_rate[CLOCK_ID_XCPU]);

	for (i = 0; periph_clk_init_table[i].periph_id != -1; i++) {
		enum periph_id periph_id;
		enum clock_id parent;
		int source, mux_bits, divider_bits;

		periph_id = periph_clk_init_table[i].periph_id;
		parent = periph_clk_init_table[i].parent_clock_id;

		source = get_periph_clock_source(periph_id, parent, &mux_bits,
						 &divider_bits);
		clock_ll_set_source_bits(periph_id, mux_bits, source);
	}
}

static void set_avp_clock_source(u32 src)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 val;

	val = (src << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT) |
		(src << SCLK_SWAKEUP_IRQ_SOURCE_SHIFT) |
		(src << SCLK_SWAKEUP_RUN_SOURCE_SHIFT) |
		(src << SCLK_SWAKEUP_IDLE_SOURCE_SHIFT) |
		(SCLK_SYS_STATE_RUN << SCLK_SYS_STATE_SHIFT);
	writel(val, &clkrst->crc_sclk_brst_pol);
	udelay(3);
}

/*
 * This function is useful on Tegra30, and any later SoCs that have compatible
 * PLLP configuration registers.
 * NOTE: Not used on Tegra210 - see tegra210_setup_pllp in T210 clock.c
 */
void tegra30_set_up_pllp(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	/*
	 * Based on the Tegra TRM, the system clock (which is the AVP clock) can
	 * run up to 275MHz. On power on, the default sytem clock source is set
	 * to PLLP_OUT0. This function sets PLLP's (hence PLLP_OUT0's) rate to
	 * 408MHz which is beyond system clock's upper limit.
	 *
	 * The fix is to set the system clock to CLK_M before initializing PLLP,
	 * and then switch back to PLLP_OUT4, which has an appropriate divider
	 * configured, after PLLP has been configured
	 */
	set_avp_clock_source(SCLK_SOURCE_CLKM);

	/*
	 * PLLP output frequency set to 408Mhz
	 * PLLC output frequency set to 228Mhz
	 */
	switch (clock_get_osc_freq()) {
	case CLOCK_OSC_FREQ_12_0: /* OSC is 12Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 408, 12, 0, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 456, 12, 1, 8);
		break;

	case CLOCK_OSC_FREQ_26_0: /* OSC is 26Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 408, 26, 0, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 26, 0, 8);
		break;

	case CLOCK_OSC_FREQ_13_0: /* OSC is 13Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 408, 13, 0, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 13, 0, 8);
		break;
	case CLOCK_OSC_FREQ_19_2:
	default:
		/*
		 * These are not supported. It is too early to print a
		 * message and the UART likely won't work anyway due to the
		 * oscillator being wrong.
		 */
		break;
	}

	/* Set PLLP_OUT1, 2, 3 & 4 freqs to 9.6, 48, 102 & 204MHz */

	/* OUT1, 2 */
	/* Assert RSTN before enable */
	reg = PLLP_OUT2_RSTN_EN | PLLP_OUT1_RSTN_EN;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[0]);
	/* Set divisor and reenable */
	reg = (IN_408_OUT_48_DIVISOR << PLLP_OUT2_RATIO)
		| PLLP_OUT2_OVR | PLLP_OUT2_CLKEN | PLLP_OUT2_RSTN_DIS
		| (IN_408_OUT_9_6_DIVISOR << PLLP_OUT1_RATIO)
		| PLLP_OUT1_OVR | PLLP_OUT1_CLKEN | PLLP_OUT1_RSTN_DIS;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[0]);

	/* OUT3, 4 */
	/* Assert RSTN before enable */
	reg = PLLP_OUT4_RSTN_EN | PLLP_OUT3_RSTN_EN;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[1]);
	/* Set divisor and reenable */
	reg = (IN_408_OUT_204_DIVISOR << PLLP_OUT4_RATIO)
		| PLLP_OUT4_OVR | PLLP_OUT4_CLKEN | PLLP_OUT4_RSTN_DIS
		| (IN_408_OUT_102_DIVISOR << PLLP_OUT3_RATIO)
		| PLLP_OUT3_OVR | PLLP_OUT3_CLKEN | PLLP_OUT3_RSTN_DIS;
	writel(reg, &clkrst->crc_pll[CLOCK_ID_PERIPH].pll_out[1]);

	set_avp_clock_source(SCLK_SOURCE_PLLP_OUT4);
}

int clock_external_output(int clk_id)
{
	u32 val;

	if (clk_id >= 1 && clk_id <= 3) {
		val = tegra_pmc_readl(offsetof(struct pmc_ctlr,
				      pmc_clk_out_cntrl));
		val |= 1 << (2 + (clk_id - 1) * 8);
		tegra_pmc_writel(val,
				 offsetof(struct pmc_ctlr,
				 pmc_clk_out_cntrl));

	} else {
		printf("%s: Unknown output clock id %d\n", __func__, clk_id);
		return -EINVAL;
	}

	return 0;
}

__weak bool clock_early_init_done(void)
{
	return true;
}
