// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk322x.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rk3228-cru.h>
#include <linux/log2.h>

enum {
	VCO_MAX_HZ	= 3200U * 1000000,
	VCO_MIN_HZ	= 800 * 1000000,
	OUTPUT_MAX_HZ	= 3200U * 1000000,
	OUTPUT_MIN_HZ	= 24 * 1000000,
};

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _refdiv, _postdiv1, _postdiv2) {\
	.refdiv = _refdiv,\
	.fbdiv = (u32)((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ), \
	.postdiv1 = _postdiv1, .postdiv2 = _postdiv2};\
	_Static_assert(((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ) * \
			 OSC_HZ / (_refdiv * _postdiv1 * _postdiv2) == hz, \
			 #hz "Hz cannot be hit with PLL "\
			 "divisors on line " __stringify(__LINE__));

/* use integer mode*/
static const struct pll_div apll_init_cfg = PLL_DIVISORS(APLL_HZ, 1, 3, 1);
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 2, 2, 1);

static int rkclk_set_pll(struct rk322x_cru *cru, enum rk_clk_id clk_id,
			 const struct pll_div *div)
{
	int pll_id = rk_pll_id(clk_id);
	struct rk322x_pll *pll = &cru->pll[pll_id];

	/* All PLLs have same VCO and output frequency range restrictions. */
	uint vco_hz = OSC_HZ / 1000 * div->fbdiv / div->refdiv * 1000;
	uint output_hz = vco_hz / div->postdiv1 / div->postdiv2;

	debug("PLL at %p: fb=%d, ref=%d, pst1=%d, pst2=%d, vco=%u Hz, output=%u Hz\n",
	      pll, div->fbdiv, div->refdiv, div->postdiv1,
	      div->postdiv2, vco_hz, output_hz);
	assert(vco_hz >= VCO_MIN_HZ && vco_hz <= VCO_MAX_HZ &&
	       output_hz >= OUTPUT_MIN_HZ && output_hz <= OUTPUT_MAX_HZ);

	/* use integer mode */
	rk_setreg(&pll->con1, 1 << PLL_DSMPD_SHIFT);
	/* Power down */
	rk_setreg(&pll->con1, 1 << PLL_PD_SHIFT);

	rk_clrsetreg(&pll->con0,
		     PLL_POSTDIV1_MASK | PLL_FBDIV_MASK,
		     (div->postdiv1 << PLL_POSTDIV1_SHIFT) | div->fbdiv);
	rk_clrsetreg(&pll->con1, PLL_POSTDIV2_MASK | PLL_REFDIV_MASK,
		     (div->postdiv2 << PLL_POSTDIV2_SHIFT |
		     div->refdiv << PLL_REFDIV_SHIFT));

	/* Power Up */
	rk_clrreg(&pll->con1, 1 << PLL_PD_SHIFT);

	/* waiting for pll lock */
	while (readl(&pll->con1) & (1 << PLL_LOCK_STATUS_SHIFT))
		udelay(1);

	return 0;
}

static void rkclk_init(struct rk322x_cru *cru)
{
	u32 aclk_div;
	u32 hclk_div;
	u32 pclk_div;

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK | APLL_MODE_MASK,
		     GPLL_MODE_SLOW << GPLL_MODE_SHIFT |
		     APLL_MODE_SLOW << APLL_MODE_SHIFT);

	/* init pll */
	rkclk_set_pll(cru, CLK_ARM, &apll_init_cfg);
	rkclk_set_pll(cru, CLK_GENERAL, &gpll_init_cfg);

	/*
	 * select apll as cpu/core clock pll source and
	 * set up dependent divisors for PERI and ACLK clocks.
	 * core hz : apll = 1:1
	 */
	aclk_div = APLL_HZ / CORE_ACLK_HZ - 1;
	assert((aclk_div + 1) * CORE_ACLK_HZ == APLL_HZ && aclk_div < 0x7);

	pclk_div = APLL_HZ / CORE_PERI_HZ - 1;
	assert((pclk_div + 1) * CORE_PERI_HZ == APLL_HZ && pclk_div < 0xf);

	rk_clrsetreg(&cru->cru_clksel_con[0],
		     CORE_CLK_PLL_SEL_MASK | CORE_DIV_CON_MASK,
		     CORE_CLK_PLL_SEL_APLL << CORE_CLK_PLL_SEL_SHIFT |
		     0 << CORE_DIV_CON_SHIFT);

	rk_clrsetreg(&cru->cru_clksel_con[1],
		     CORE_ACLK_DIV_MASK | CORE_PERI_DIV_MASK,
		     aclk_div << CORE_ACLK_DIV_SHIFT |
		     pclk_div << CORE_PERI_DIV_SHIFT);

	/*
	 * select gpll as pd_bus bus clock source and
	 * set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = GPLL_HZ / BUS_ACLK_HZ - 1;
	assert((aclk_div + 1) * BUS_ACLK_HZ == GPLL_HZ && aclk_div <= 0x1f);

	pclk_div = BUS_ACLK_HZ / BUS_PCLK_HZ - 1;
	assert((pclk_div + 1) * BUS_PCLK_HZ == BUS_ACLK_HZ && pclk_div <= 0x7);

	hclk_div = BUS_ACLK_HZ / BUS_HCLK_HZ - 1;
	assert((hclk_div + 1) * BUS_HCLK_HZ == BUS_ACLK_HZ && hclk_div <= 0x3);

	rk_clrsetreg(&cru->cru_clksel_con[0],
		     BUS_ACLK_PLL_SEL_MASK | BUS_ACLK_DIV_MASK,
		     BUS_ACLK_PLL_SEL_GPLL << BUS_ACLK_PLL_SEL_SHIFT |
		     aclk_div << BUS_ACLK_DIV_SHIFT);

	rk_clrsetreg(&cru->cru_clksel_con[1],
		     BUS_PCLK_DIV_MASK | BUS_HCLK_DIV_MASK,
		     pclk_div << BUS_PCLK_DIV_SHIFT |
		     hclk_div << BUS_HCLK_DIV_SHIFT);

	/*
	 * select gpll as pd_peri bus clock source and
	 * set up dependent divisors for PCLK/HCLK and ACLK clocks.
	 */
	aclk_div = GPLL_HZ / PERI_ACLK_HZ - 1;
	assert((aclk_div + 1) * PERI_ACLK_HZ == GPLL_HZ && aclk_div < 0x1f);

	hclk_div = ilog2(PERI_ACLK_HZ / PERI_HCLK_HZ);
	assert((1 << hclk_div) * PERI_HCLK_HZ ==
		PERI_ACLK_HZ && (hclk_div < 0x4));

	pclk_div = ilog2(PERI_ACLK_HZ / PERI_PCLK_HZ);
	assert((1 << pclk_div) * PERI_PCLK_HZ ==
		PERI_ACLK_HZ && pclk_div < 0x8);

	rk_clrsetreg(&cru->cru_clksel_con[10],
		     PERI_PLL_SEL_MASK | PERI_PCLK_DIV_MASK |
		     PERI_HCLK_DIV_MASK | PERI_ACLK_DIV_MASK,
		     PERI_PLL_GPLL << PERI_PLL_SEL_SHIFT |
		     pclk_div << PERI_PCLK_DIV_SHIFT |
		     hclk_div << PERI_HCLK_DIV_SHIFT |
		     aclk_div << PERI_ACLK_DIV_SHIFT);

	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con,
		     GPLL_MODE_MASK | APLL_MODE_MASK,
		     GPLL_MODE_NORM << GPLL_MODE_SHIFT |
		     APLL_MODE_NORM << APLL_MODE_SHIFT);
}

/* Get pll rate by id */
static uint32_t rkclk_pll_get_rate(struct rk322x_cru *cru,
				   enum rk_clk_id clk_id)
{
	uint32_t refdiv, fbdiv, postdiv1, postdiv2;
	uint32_t con;
	int pll_id = rk_pll_id(clk_id);
	struct rk322x_pll *pll = &cru->pll[pll_id];
	static u8 clk_shift[CLK_COUNT] = {
		0xff, APLL_MODE_SHIFT, DPLL_MODE_SHIFT, 0xff,
		GPLL_MODE_SHIFT, 0xff
	};
	static u32 clk_mask[CLK_COUNT] = {
		0xff, APLL_MODE_MASK, DPLL_MODE_MASK, 0xff,
		GPLL_MODE_MASK, 0xff
	};
	uint shift;
	uint mask;

	con = readl(&cru->cru_mode_con);
	shift = clk_shift[clk_id];
	mask = clk_mask[clk_id];

	switch ((con & mask) >> shift) {
	case GPLL_MODE_SLOW:
		return OSC_HZ;
	case GPLL_MODE_NORM:

		/* normal mode */
		con = readl(&pll->con0);
		postdiv1 = (con & PLL_POSTDIV1_MASK) >> PLL_POSTDIV1_SHIFT;
		fbdiv = (con & PLL_FBDIV_MASK) >> PLL_FBDIV_SHIFT;
		con = readl(&pll->con1);
		postdiv2 = (con & PLL_POSTDIV2_MASK) >> PLL_POSTDIV2_SHIFT;
		refdiv = (con & PLL_REFDIV_MASK) >> PLL_REFDIV_SHIFT;
		return (24 * fbdiv / (refdiv * postdiv1 * postdiv2)) * 1000000;
	default:
		return 32768;
	}
}

static ulong rockchip_mmc_get_clk(struct rk322x_cru *cru, uint clk_general_rate,
				  int periph)
{
	uint src_rate;
	uint div, mux;
	u32 con;

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
	case SCLK_EMMC_SAMPLE:
		con = readl(&cru->cru_clksel_con[11]);
		mux = (con & EMMC_PLL_MASK) >> EMMC_PLL_SHIFT;
		con = readl(&cru->cru_clksel_con[12]);
		div = (con & EMMC_DIV_MASK) >> EMMC_DIV_SHIFT;
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		con = readl(&cru->cru_clksel_con[11]);
		mux = (con & MMC0_PLL_MASK) >> MMC0_PLL_SHIFT;
		div = (con & MMC0_DIV_MASK) >> MMC0_DIV_SHIFT;
		break;
	default:
		return -EINVAL;
	}

	src_rate = mux == EMMC_SEL_24M ? OSC_HZ : clk_general_rate;
	return DIV_TO_RATE(src_rate, div) / 2;
}

static ulong rk322x_mac_set_clk(struct rk322x_cru *cru, uint freq)
{
	ulong ret;

	/*
	 * The gmac clock can be derived either from an external clock
	 * or can be generated from internally by a divider from SCLK_MAC.
	 */
	if (readl(&cru->cru_clksel_con[5]) & BIT(5)) {
		/* An external clock will always generate the right rate... */
		ret = freq;
	} else {
		u32 con = readl(&cru->cru_clksel_con[5]);
		ulong pll_rate;
		u8 div;

		if ((con >> MAC_PLL_SEL_SHIFT) & MAC_PLL_SEL_MASK)
			pll_rate = GPLL_HZ;
		else
			/* CPLL is not set */
			return -EPERM;

		div = DIV_ROUND_UP(pll_rate, freq) - 1;
		if (div <= 0x1f)
			rk_clrsetreg(&cru->cru_clksel_con[5], CLK_MAC_DIV_MASK,
				     div << CLK_MAC_DIV_SHIFT);
		else
			debug("Unsupported div for gmac:%d\n", div);

		return DIV_TO_RATE(pll_rate, div);
	}

	return ret;
}

static ulong rockchip_mmc_set_clk(struct rk322x_cru *cru, uint clk_general_rate,
				  int periph, uint freq)
{
	int src_clk_div;
	int mux;

	debug("%s: clk_general_rate=%u\n", __func__, clk_general_rate);

	/* mmc clock defaulg div 2 internal, need provide double in cru */
	src_clk_div = DIV_ROUND_UP(clk_general_rate / 2, freq);

	if (src_clk_div > 128) {
		src_clk_div = DIV_ROUND_UP(OSC_HZ / 2, freq);
		assert(src_clk_div - 1 < 128);
		mux = EMMC_SEL_24M;
	} else {
		mux = EMMC_SEL_GPLL;
	}

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
	case SCLK_EMMC_SAMPLE:
		rk_clrsetreg(&cru->cru_clksel_con[11],
			     EMMC_PLL_MASK,
			     mux << EMMC_PLL_SHIFT);
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     EMMC_DIV_MASK,
			     (src_clk_div - 1) << EMMC_DIV_SHIFT);
		break;
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		rk_clrsetreg(&cru->cru_clksel_con[11],
			     MMC0_PLL_MASK | MMC0_DIV_MASK,
			     mux << MMC0_PLL_SHIFT |
			     (src_clk_div - 1) << MMC0_DIV_SHIFT);
		break;
	default:
		return -EINVAL;
	}

	return rockchip_mmc_get_clk(cru, clk_general_rate, periph);
}

static int rk322x_ddr_set_clk(struct rk322x_cru *cru, unsigned int set_rate)
{
	struct pll_div dpll_cfg;

	/*  clk_ddrc == DPLL = 24MHz / refdiv * fbdiv / postdiv1 / postdiv2 */
	switch (set_rate) {
	case 400*MHz:
		dpll_cfg = (struct pll_div)
		{.refdiv = 1, .fbdiv = 50, .postdiv1 = 3, .postdiv2 = 1};
		break;
	case 600*MHz:
		dpll_cfg = (struct pll_div)
		{.refdiv = 1, .fbdiv = 75, .postdiv1 = 3, .postdiv2 = 1};
		break;
	case 800*MHz:
		dpll_cfg = (struct pll_div)
		{.refdiv = 1, .fbdiv = 100, .postdiv1 = 3, .postdiv2 = 1};
		break;
	}

	/* pll enter slow-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK,
		     DPLL_MODE_SLOW << DPLL_MODE_SHIFT);
	rkclk_set_pll(cru, CLK_DDR, &dpll_cfg);
	/* PLL enter normal-mode */
	rk_clrsetreg(&cru->cru_mode_con, DPLL_MODE_MASK,
		     DPLL_MODE_NORM << DPLL_MODE_SHIFT);

	return set_rate;
}
static ulong rk322x_clk_get_rate(struct clk *clk)
{
	struct rk322x_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate, gclk_rate;

	gclk_rate = rkclk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case 0 ... 63:
		rate = rkclk_pll_get_rate(priv->cru, clk->id);
		break;
	case HCLK_EMMC:
	case SCLK_EMMC:
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		rate = rockchip_mmc_get_clk(priv->cru, gclk_rate, clk->id);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk322x_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk322x_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate, gclk_rate;

	gclk_rate = rkclk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case HCLK_EMMC:
	case SCLK_EMMC:
	case HCLK_SDMMC:
	case SCLK_SDMMC:
		new_rate = rockchip_mmc_set_clk(priv->cru, gclk_rate,
						clk->id, rate);
		break;
	case CLK_DDR:
		new_rate = rk322x_ddr_set_clk(priv->cru, rate);
		break;
	case SCLK_MAC:
		new_rate = rk322x_mac_set_clk(priv->cru, rate);
		break;
	case PLL_GPLL:
		return 0;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static int rk322x_gmac_set_parent(struct clk *clk, struct clk *parent)
{
	struct rk322x_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk322x_cru *cru = priv->cru;

	/*
	 * If the requested parent is in the same clock-controller and the id
	 * is SCLK_MAC_SRC ("sclk_gmac_src"), switch to the internal clock.
	 */
	if ((parent->dev == clk->dev) && (parent->id == SCLK_MAC_SRC)) {
		debug("%s: switching RGMII to SCLK_MAC_SRC\n", __func__);
		rk_clrsetreg(&cru->cru_clksel_con[5], BIT(5), 0);
		return 0;
	}

	/*
	 * If the requested parent is in the same clock-controller and the id
	 * is SCLK_MAC_EXTCLK (sclk_mac_extclk), switch to the external clock.
	 */
	if ((parent->dev == clk->dev) && (parent->id == SCLK_MAC_EXTCLK)) {
		debug("%s: switching RGMII to SCLK_MAC_EXTCLK\n", __func__);
		rk_clrsetreg(&cru->cru_clksel_con[5], BIT(5), BIT(5));
		return 0;
	}

	return -EINVAL;
}

static int rk322x_gmac_extclk_set_parent(struct clk *clk, struct clk *parent)
{
	struct rk322x_clk_priv *priv = dev_get_priv(clk->dev);
	const char *clock_output_name;
	struct rk322x_cru *cru = priv->cru;
	int ret;

	ret = dev_read_string_index(parent->dev, "clock-output-names",
				    parent->id, &clock_output_name);
	if (ret < 0)
		return -ENODATA;

	if (!strcmp(clock_output_name, "ext_gmac")) {
		debug("%s: switching gmac extclk to ext_gmac\n", __func__);
		rk_clrsetreg(&cru->cru_clksel_con[29], BIT(10), 0);
		return 0;
	} else if (!strcmp(clock_output_name, "phy_50m_out")) {
		debug("%s: switching gmac extclk to phy_50m_out\n", __func__);
		rk_clrsetreg(&cru->cru_clksel_con[29], BIT(10), BIT(10));
		return 0;
	}

	return -EINVAL;
}

static int rk322x_clk_set_parent(struct clk *clk, struct clk *parent)
{
	switch (clk->id) {
	case SCLK_MAC:
		return rk322x_gmac_set_parent(clk, parent);
	case SCLK_MAC_EXTCLK:
		return rk322x_gmac_extclk_set_parent(clk, parent);
	}

	debug("%s: unsupported clk %ld\n", __func__, clk->id);
	return -ENOENT;
}

static struct clk_ops rk322x_clk_ops = {
	.get_rate	= rk322x_clk_get_rate,
	.set_rate	= rk322x_clk_set_rate,
	.set_parent	= rk322x_clk_set_parent,
};

static int rk322x_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk322x_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rk322x_clk_probe(struct udevice *dev)
{
	struct rk322x_clk_priv *priv = dev_get_priv(dev);

	rkclk_init(priv->cru);

	return 0;
}

static int rk322x_clk_bind(struct udevice *dev)
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
		priv->glb_srst_fst_value = offsetof(struct rk322x_cru,
						    cru_glb_srst_fst_value);
		priv->glb_srst_snd_value = offsetof(struct rk322x_cru,
						    cru_glb_srst_snd_value);
		sys_child->priv = priv;
	}

#if CONFIG_IS_ENABLED(CONFIG_RESET_ROCKCHIP)
	ret = offsetof(struct rk322x_cru, cru_softrst_con[0]);
	ret = rockchip_reset_bind(dev, ret, 9);
	if (ret)
		debug("Warning: software reset driver bind faile\n");
#endif

	return 0;
}

static const struct udevice_id rk322x_clk_ids[] = {
	{ .compatible = "rockchip,rk3228-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk322x_cru) = {
	.name		= "clk_rk322x",
	.id		= UCLASS_CLK,
	.of_match	= rk322x_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rk322x_clk_priv),
	.ofdata_to_platdata = rk322x_clk_ofdata_to_platdata,
	.ops		= &rk322x_clk_ops,
	.bind		= rk322x_clk_bind,
	.probe		= rk322x_clk_probe,
};
