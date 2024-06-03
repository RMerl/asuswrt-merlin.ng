// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-structs.h>
#include <errno.h>
#include <mapmem.h>
#include <syscon.h>
#include <bitfield.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3368.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rk3368-cru.h>

#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct rk3368_clk_plat {
	struct dtd_rockchip_rk3368_cru dtd;
};
#endif

struct pll_div {
	u32 nr;
	u32 nf;
	u32 no;
};

#define OSC_HZ		(24 * 1000 * 1000)
#define APLL_L_HZ	(800 * 1000 * 1000)
#define APLL_B_HZ	(816 * 1000 * 1000)
#define GPLL_HZ		(576 * 1000 * 1000)
#define CPLL_HZ		(400 * 1000 * 1000)

#define DIV_TO_RATE(input_rate, div)    ((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _nr, _no) { \
	.nr = _nr, .nf = (u32)((u64)hz * _nr * _no / OSC_HZ), .no = _no}; \
	_Static_assert(((u64)hz * _nr * _no / OSC_HZ) * OSC_HZ /\
		       (_nr * _no) == hz, #hz "Hz cannot be hit with PLL " \
		       "divisors on line " __stringify(__LINE__));

#if IS_ENABLED(CONFIG_SPL_BUILD) || IS_ENABLED(CONFIG_TPL_BUILD)
static const struct pll_div apll_l_init_cfg = PLL_DIVISORS(APLL_L_HZ, 12, 2);
static const struct pll_div apll_b_init_cfg = PLL_DIVISORS(APLL_B_HZ, 1, 2);
#if !defined(CONFIG_TPL_BUILD)
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 1, 2);
static const struct pll_div cpll_init_cfg = PLL_DIVISORS(CPLL_HZ, 1, 6);
#endif
#endif

static ulong rk3368_clk_get_rate(struct clk *clk);

/* Get pll rate by id */
static uint32_t rkclk_pll_get_rate(struct rk3368_cru *cru,
				   enum rk3368_pll_id pll_id)
{
	uint32_t nr, no, nf;
	uint32_t con;
	struct rk3368_pll *pll = &cru->pll[pll_id];

	con = readl(&pll->con3);

	switch ((con & PLL_MODE_MASK) >> PLL_MODE_SHIFT) {
	case PLL_MODE_SLOW:
		return OSC_HZ;
	case PLL_MODE_NORMAL:
		con = readl(&pll->con0);
		no = ((con & PLL_OD_MASK) >> PLL_OD_SHIFT) + 1;
		nr = ((con & PLL_NR_MASK) >> PLL_NR_SHIFT) + 1;
		con = readl(&pll->con1);
		nf = ((con & PLL_NF_MASK) >> PLL_NF_SHIFT) + 1;

		return (24 * nf / (nr * no)) * 1000000;
	case PLL_MODE_DEEP_SLOW:
	default:
		return 32768;
	}
}

#if IS_ENABLED(CONFIG_SPL_BUILD) || IS_ENABLED(CONFIG_TPL_BUILD)
static int rkclk_set_pll(struct rk3368_cru *cru, enum rk3368_pll_id pll_id,
			 const struct pll_div *div)
{
	struct rk3368_pll *pll = &cru->pll[pll_id];
	/* All PLLs have same VCO and output frequency range restrictions*/
	uint vco_hz = OSC_HZ / 1000 * div->nf / div->nr * 1000;
	uint output_hz = vco_hz / div->no;

	debug("PLL at %p: nf=%d, nr=%d, no=%d, vco=%u Hz, output=%u Hz\n",
	      pll, div->nf, div->nr, div->no, vco_hz, output_hz);

	/* enter slow mode and reset pll */
	rk_clrsetreg(&pll->con3, PLL_MODE_MASK | PLL_RESET_MASK,
		     PLL_RESET << PLL_RESET_SHIFT);

	rk_clrsetreg(&pll->con0, PLL_NR_MASK | PLL_OD_MASK,
		     ((div->nr - 1) << PLL_NR_SHIFT) |
		     ((div->no - 1) << PLL_OD_SHIFT));
	writel((div->nf - 1) << PLL_NF_SHIFT, &pll->con1);
	/*
	 * BWADJ should be set to NF / 2 to ensure the nominal bandwidth.
	 * Compare the RK3368 TRM, section "3.6.4 PLL Bandwidth Adjustment".
	 */
	clrsetbits_le32(&pll->con2, PLL_BWADJ_MASK, (div->nf >> 1) - 1);

	udelay(10);

	/* return from reset */
	rk_clrreg(&pll->con3, PLL_RESET_MASK);

	/* waiting for pll lock */
	while (!(readl(&pll->con1) & PLL_LOCK_STA))
		udelay(1);

	rk_clrsetreg(&pll->con3, PLL_MODE_MASK,
		     PLL_MODE_NORMAL << PLL_MODE_SHIFT);

	return 0;
}
#endif

#if IS_ENABLED(CONFIG_SPL_BUILD) || IS_ENABLED(CONFIG_TPL_BUILD)
static void rkclk_init(struct rk3368_cru *cru)
{
	u32 apllb, aplll, dpll, cpll, gpll;

	rkclk_set_pll(cru, APLLB, &apll_b_init_cfg);
	rkclk_set_pll(cru, APLLL, &apll_l_init_cfg);
#if !defined(CONFIG_TPL_BUILD)
	/*
	 * If we plan to return to the boot ROM, we can't increase the
	 * GPLL rate from the SPL stage.
	 */
	rkclk_set_pll(cru, GPLL, &gpll_init_cfg);
	rkclk_set_pll(cru, CPLL, &cpll_init_cfg);
#endif

	apllb = rkclk_pll_get_rate(cru, APLLB);
	aplll = rkclk_pll_get_rate(cru, APLLL);
	dpll = rkclk_pll_get_rate(cru, DPLL);
	cpll = rkclk_pll_get_rate(cru, CPLL);
	gpll = rkclk_pll_get_rate(cru, GPLL);

	debug("%s apllb(%d) apll(%d) dpll(%d) cpll(%d) gpll(%d)\n",
	       __func__, apllb, aplll, dpll, cpll, gpll);
}
#endif

#if !IS_ENABLED(CONFIG_SPL_BUILD) || CONFIG_IS_ENABLED(MMC_SUPPORT)
static ulong rk3368_mmc_get_clk(struct rk3368_cru *cru, uint clk_id)
{
	u32 div, con, con_id, rate;
	u32 pll_rate;

	switch (clk_id) {
	case HCLK_SDMMC:
		con_id = 50;
		break;
	case HCLK_EMMC:
		con_id = 51;
		break;
	case SCLK_SDIO0:
		con_id = 48;
		break;
	default:
		return -EINVAL;
	}

	con = readl(&cru->clksel_con[con_id]);
	switch (con & MMC_PLL_SEL_MASK) {
	case MMC_PLL_SEL_GPLL:
		pll_rate = rkclk_pll_get_rate(cru, GPLL);
		break;
	case MMC_PLL_SEL_24M:
		pll_rate = OSC_HZ;
		break;
	case MMC_PLL_SEL_CPLL:
		pll_rate = rkclk_pll_get_rate(cru, CPLL);
		break;
	case MMC_PLL_SEL_USBPHY_480M:
	default:
		return -EINVAL;
	}
	div = (con & MMC_CLK_DIV_MASK) >> MMC_CLK_DIV_SHIFT;
	rate = DIV_TO_RATE(pll_rate, div);

	debug("%s: raw rate %d (post-divide by 2)\n", __func__, rate);
	return rate >> 1;
}

static ulong rk3368_mmc_find_best_rate_and_parent(struct clk *clk,
						  ulong rate,
						  u32 *best_mux,
						  u32 *best_div)
{
	int i;
	ulong best_rate = 0;
	const ulong MHz = 1000000;
	const struct {
		u32 mux;
		ulong rate;
	} parents[] = {
		{ .mux = MMC_PLL_SEL_CPLL, .rate = CPLL_HZ },
		{ .mux = MMC_PLL_SEL_GPLL, .rate = GPLL_HZ },
		{ .mux = MMC_PLL_SEL_24M,  .rate = 24 * MHz }
	};

	debug("%s: target rate %ld\n", __func__, rate);
	for (i = 0; i < ARRAY_SIZE(parents); ++i) {
		/*
		 * Find the largest rate no larger than the target-rate for
		 * the current parent.
		 */
		ulong parent_rate = parents[i].rate;
		u32 div = DIV_ROUND_UP(parent_rate, rate);
		u32 adj_div = div;
		ulong new_rate = parent_rate / adj_div;

		debug("%s: rate %ld, parent-mux %d, parent-rate %ld, div %d\n",
		      __func__, rate, parents[i].mux, parents[i].rate, div);

		/* Skip, if not representable */
		if ((div - 1) > MMC_CLK_DIV_MASK)
			continue;

		/* Skip, if we already have a better (or equal) solution */
		if (new_rate <= best_rate)
			continue;

		/* This is our new best rate. */
		best_rate = new_rate;
		*best_mux = parents[i].mux;
		*best_div = div - 1;
	}

	debug("%s: best_mux = %x, best_div = %d, best_rate = %ld\n",
	      __func__, *best_mux, *best_div, best_rate);

	return best_rate;
}

static ulong rk3368_mmc_set_clk(struct clk *clk, ulong rate)
{
	struct rk3368_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3368_cru *cru = priv->cru;
	ulong clk_id = clk->id;
	u32 con_id, mux = 0, div = 0;

	/* Find the best parent and rate */
	rk3368_mmc_find_best_rate_and_parent(clk, rate << 1, &mux, &div);

	switch (clk_id) {
	case HCLK_SDMMC:
		con_id = 50;
		break;
	case HCLK_EMMC:
		con_id = 51;
		break;
	case SCLK_SDIO0:
		con_id = 48;
		break;
	default:
		return -EINVAL;
	}

	rk_clrsetreg(&cru->clksel_con[con_id],
		     MMC_PLL_SEL_MASK | MMC_CLK_DIV_MASK,
		     mux | div);

	return rk3368_mmc_get_clk(cru, clk_id);
}
#endif

#if IS_ENABLED(CONFIG_TPL_BUILD)
static ulong rk3368_ddr_set_clk(struct rk3368_cru *cru, ulong set_rate)
{
	const struct pll_div *dpll_cfg = NULL;
	const ulong MHz = 1000000;

	/* Fout = ((Fin /NR) * NF )/ NO */
	static const struct pll_div dpll_1200 = PLL_DIVISORS(1200 * MHz, 1, 1);
	static const struct pll_div dpll_1332 =	PLL_DIVISORS(1332 * MHz, 2, 1);
	static const struct pll_div dpll_1600 =	PLL_DIVISORS(1600 * MHz, 3, 2);

	switch (set_rate) {
	case 1200*MHz:
		dpll_cfg = &dpll_1200;
		break;
	case 1332*MHz:
		dpll_cfg = &dpll_1332;
		break;
	case 1600*MHz:
		dpll_cfg = &dpll_1600;
		break;
	default:
		pr_err("Unsupported SDRAM frequency!,%ld\n", set_rate);
	}
	rkclk_set_pll(cru, DPLL, dpll_cfg);

	return set_rate;
}
#endif

#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
static ulong rk3368_gmac_set_clk(struct rk3368_cru *cru, ulong set_rate)
{
	ulong ret;

	/*
	 * The gmac clock can be derived either from an external clock
	 * or can be generated from internally by a divider from SCLK_MAC.
	 */
	if (readl(&cru->clksel_con[43]) & GMAC_MUX_SEL_EXTCLK) {
		/* An external clock will always generate the right rate... */
		ret = set_rate;
	} else {
		u32 con = readl(&cru->clksel_con[43]);
		ulong pll_rate;
		u8 div;

		if (((con >> GMAC_PLL_SHIFT) & GMAC_PLL_MASK) ==
		    GMAC_PLL_SELECT_GENERAL)
			pll_rate = GPLL_HZ;
		else if (((con >> GMAC_PLL_SHIFT) & GMAC_PLL_MASK) ==
			 GMAC_PLL_SELECT_CODEC)
			pll_rate = CPLL_HZ;
		else
			/* CPLL is not set */
			return -EPERM;

		div = DIV_ROUND_UP(pll_rate, set_rate) - 1;
		if (div <= 0x1f)
			rk_clrsetreg(&cru->clksel_con[43], GMAC_DIV_CON_MASK,
				     div << GMAC_DIV_CON_SHIFT);
		else
			debug("Unsupported div for gmac:%d\n", div);

		return DIV_TO_RATE(pll_rate, div);
	}

	return ret;
}
#endif

/*
 * RK3368 SPI clocks have a common divider-width (7 bits) and a single bit
 * to select either CPLL or GPLL as the clock-parent. The location within
 * the enclosing CLKSEL_CON (i.e. div_shift and sel_shift) are variable.
 */

struct spi_clkreg {
	uint8_t reg;  /* CLKSEL_CON[reg] register in CRU */
	uint8_t div_shift;
	uint8_t sel_shift;
};

/*
 * The entries are numbered relative to their offset from SCLK_SPI0.
 */
static const struct spi_clkreg spi_clkregs[] = {
	[0] = { .reg = 45, .div_shift = 0, .sel_shift = 7, },
	[1] = { .reg = 45, .div_shift = 8, .sel_shift = 15, },
	[2] = { .reg = 46, .div_shift = 8, .sel_shift = 15, },
};

static inline u32 extract_bits(u32 val, unsigned width, unsigned shift)
{
	return (val >> shift) & ((1 << width) - 1);
}

static ulong rk3368_spi_get_clk(struct rk3368_cru *cru, ulong clk_id)
{
	const struct spi_clkreg *spiclk = NULL;
	u32 div, val;

	switch (clk_id) {
	case SCLK_SPI0 ... SCLK_SPI2:
		spiclk = &spi_clkregs[clk_id - SCLK_SPI0];
		break;

	default:
		pr_err("%s: SPI clk-id %ld not supported\n", __func__, clk_id);
		return -EINVAL;
	}

	val = readl(&cru->clksel_con[spiclk->reg]);
	div = extract_bits(val, 7, spiclk->div_shift);

	debug("%s: div 0x%x\n", __func__, div);
	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rk3368_spi_set_clk(struct rk3368_cru *cru, ulong clk_id, uint hz)
{
	const struct spi_clkreg *spiclk = NULL;
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(GPLL_HZ, hz);
	assert(src_clk_div < 127);

	switch (clk_id) {
	case SCLK_SPI0 ... SCLK_SPI2:
		spiclk = &spi_clkregs[clk_id - SCLK_SPI0];
		break;

	default:
		pr_err("%s: SPI clk-id %ld not supported\n", __func__, clk_id);
		return -EINVAL;
	}

	rk_clrsetreg(&cru->clksel_con[spiclk->reg],
		     ((0x7f << spiclk->div_shift) |
		      (0x1 << spiclk->sel_shift)),
		     ((src_clk_div << spiclk->div_shift) |
		      (1 << spiclk->sel_shift)));

	return rk3368_spi_get_clk(cru, clk_id);
}

static ulong rk3368_saradc_get_clk(struct rk3368_cru *cru)
{
	u32 div, val;

	val = readl(&cru->clksel_con[25]);
	div = bitfield_extract(val, CLK_SARADC_DIV_CON_SHIFT,
			       CLK_SARADC_DIV_CON_WIDTH);

	return DIV_TO_RATE(OSC_HZ, div);
}

static ulong rk3368_saradc_set_clk(struct rk3368_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(OSC_HZ, hz) - 1;
	assert(src_clk_div < 128);

	rk_clrsetreg(&cru->clksel_con[25],
		     CLK_SARADC_DIV_CON_MASK,
		     src_clk_div << CLK_SARADC_DIV_CON_SHIFT);

	return rk3368_saradc_get_clk(cru);
}

static ulong rk3368_clk_get_rate(struct clk *clk)
{
	struct rk3368_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	debug("%s: id %ld\n", __func__, clk->id);
	switch (clk->id) {
	case PLL_CPLL:
		rate = rkclk_pll_get_rate(priv->cru, CPLL);
		break;
	case PLL_GPLL:
		rate = rkclk_pll_get_rate(priv->cru, GPLL);
		break;
	case SCLK_SPI0 ... SCLK_SPI2:
		rate = rk3368_spi_get_clk(priv->cru, clk->id);
		break;
#if !IS_ENABLED(CONFIG_SPL_BUILD) || CONFIG_IS_ENABLED(MMC_SUPPORT)
	case HCLK_SDMMC:
	case HCLK_EMMC:
		rate = rk3368_mmc_get_clk(priv->cru, clk->id);
		break;
#endif
	case SCLK_SARADC:
		rate = rk3368_saradc_get_clk(priv->cru);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3368_clk_set_rate(struct clk *clk, ulong rate)
{
	__maybe_unused struct rk3368_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	debug("%s id:%ld rate:%ld\n", __func__, clk->id, rate);
	switch (clk->id) {
	case SCLK_SPI0 ... SCLK_SPI2:
		ret = rk3368_spi_set_clk(priv->cru, clk->id, rate);
		break;
#if IS_ENABLED(CONFIG_TPL_BUILD)
	case CLK_DDR:
		ret = rk3368_ddr_set_clk(priv->cru, rate);
		break;
#endif
#if !IS_ENABLED(CONFIG_SPL_BUILD) || CONFIG_IS_ENABLED(MMC_SUPPORT)
	case HCLK_SDMMC:
	case HCLK_EMMC:
		ret = rk3368_mmc_set_clk(clk, rate);
		break;
#endif
#if CONFIG_IS_ENABLED(GMAC_ROCKCHIP)
	case SCLK_MAC:
		/* select the external clock */
		ret = rk3368_gmac_set_clk(priv->cru, rate);
		break;
#endif
	case SCLK_SARADC:
		ret =  rk3368_saradc_set_clk(priv->cru, rate);
		break;
	default:
		return -ENOENT;
	}

	return ret;
}

static int __maybe_unused rk3368_gmac_set_parent(struct clk *clk, struct clk *parent)
{
	struct rk3368_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3368_cru *cru = priv->cru;
	const char *clock_output_name;
	int ret;

	/*
	 * If the requested parent is in the same clock-controller and
	 * the id is SCLK_MAC ("sclk_mac"), switch to the internal
	 * clock.
	 */
	if ((parent->dev == clk->dev) && (parent->id == SCLK_MAC)) {
		debug("%s: switching GAMC to SCLK_MAC\n", __func__);
		rk_clrreg(&cru->clksel_con[43], GMAC_MUX_SEL_EXTCLK);
		return 0;
	}

	/*
	 * Otherwise, we need to check the clock-output-names of the
	 * requested parent to see if the requested id is "ext_gmac".
	 */
	ret = dev_read_string_index(parent->dev, "clock-output-names",
				    parent->id, &clock_output_name);
	if (ret < 0)
		return -ENODATA;

	/* If this is "ext_gmac", switch to the external clock input */
	if (!strcmp(clock_output_name, "ext_gmac")) {
		debug("%s: switching GMAC to external clock\n", __func__);
		rk_setreg(&cru->clksel_con[43], GMAC_MUX_SEL_EXTCLK);
		return 0;
	}

	return -EINVAL;
}

static int __maybe_unused rk3368_clk_set_parent(struct clk *clk, struct clk *parent)
{
	switch (clk->id) {
	case SCLK_MAC:
		return rk3368_gmac_set_parent(clk, parent);
	}

	debug("%s: unsupported clk %ld\n", __func__, clk->id);
	return -ENOENT;
}

static int rk3368_clk_enable(struct clk *clk)
{
	switch (clk->id) {
	case SCLK_MAC:
	case SCLK_MAC_RX:
	case SCLK_MAC_TX:
	case SCLK_MACREF:
	case SCLK_MACREF_OUT:
	case ACLK_GMAC:
	case PCLK_GMAC:
		/* Required to successfully probe the Designware GMAC driver */
		return 0;
	}

	debug("%s: unsupported clk %ld\n", __func__, clk->id);
	return -ENOENT;
}

static struct clk_ops rk3368_clk_ops = {
	.get_rate = rk3368_clk_get_rate,
	.set_rate = rk3368_clk_set_rate,
#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
	.set_parent = rk3368_clk_set_parent,
#endif
	.enable = rk3368_clk_enable,
};

static int rk3368_clk_probe(struct udevice *dev)
{
	struct rk3368_clk_priv __maybe_unused *priv = dev_get_priv(dev);
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3368_clk_plat *plat = dev_get_platdata(dev);

	priv->cru = map_sysmem(plat->dtd.reg[0], plat->dtd.reg[1]);
#endif
#if IS_ENABLED(CONFIG_SPL_BUILD) || IS_ENABLED(CONFIG_TPL_BUILD)
	rkclk_init(priv->cru);
#endif

	return 0;
}

static int rk3368_clk_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rk3368_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);
#endif

	return 0;
}

static int rk3368_clk_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child;
	struct sysreset_reg *priv;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rk3368_cru,
						    glb_srst_fst_val);
		priv->glb_srst_snd_value = offsetof(struct rk3368_cru,
						    glb_srst_snd_val);
		sys_child->priv = priv;
	}

#if CONFIG_IS_ENABLED(CONFIG_RESET_ROCKCHIP)
	ret = offsetof(struct rk3368_cru, softrst_con[0]);
	ret = rockchip_reset_bind(dev, ret, 15);
	if (ret)
		debug("Warning: software reset driver bind faile\n");
#endif

	return ret;
}

static const struct udevice_id rk3368_clk_ids[] = {
	{ .compatible = "rockchip,rk3368-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3368_cru) = {
	.name		= "rockchip_rk3368_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3368_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rk3368_clk_priv),
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	.platdata_auto_alloc_size = sizeof(struct rk3368_clk_plat),
#endif
	.ofdata_to_platdata = rk3368_clk_ofdata_to_platdata,
	.ops		= &rk3368_clk_ops,
	.bind		= rk3368_clk_bind,
	.probe		= rk3368_clk_probe,
};
