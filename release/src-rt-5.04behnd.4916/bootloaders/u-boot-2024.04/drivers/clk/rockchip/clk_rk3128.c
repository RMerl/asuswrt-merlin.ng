// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3128.h>
#include <asm/arch-rockchip/hardware.h>
#include <bitfield.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rk3128-cru.h>
#include <linux/delay.h>
#include <linux/log2.h>

enum {
	VCO_MAX_HZ	= 2400U * 1000000,
	VCO_MIN_HZ	= 600 * 1000000,
	OUTPUT_MAX_HZ	= 2400U * 1000000,
	OUTPUT_MIN_HZ	= 24 * 1000000,
};

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _refdiv, _postdiv1, _postdiv2) {\
	.refdiv = _refdiv,\
	.fbdiv = (u32)((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ),\
	.postdiv1 = _postdiv1, .postdiv2 = _postdiv2};

/* use integer mode*/
static const struct pll_div apll_init_cfg = PLL_DIVISORS(APLL_HZ, 1, 3, 1);
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 2, 2, 1);

static int rkclk_set_pll(struct rk3128_cru *cru, enum rk_clk_id clk_id,
			 const struct pll_div *div)
{
	int pll_id = rk_pll_id(clk_id);
	struct rk3128_pll *pll = &cru->pll[pll_id];

	/* All PLLs have same VCO and output frequency range restrictions. */
	uint vco_hz = OSC_HZ / 1000 * div->fbdiv / div->refdiv * 1000;
	uint output_hz = vco_hz / div->postdiv1 / div->postdiv2;

	debug("PLL at %p:fd=%d,rd=%d,pd1=%d,pd2=%d,vco=%uHz,output=%uHz\n",
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

static int pll_para_config(u32 freq_hz, struct pll_div *div)
{
	u32 ref_khz = OSC_HZ / 1000, refdiv, fbdiv = 0;
	u32 postdiv1, postdiv2 = 1;
	u32 fref_khz;
	u32 diff_khz, best_diff_khz;
	const u32 max_refdiv = 63, max_fbdiv = 3200, min_fbdiv = 16;
	const u32 max_postdiv1 = 7, max_postdiv2 = 7;
	u32 vco_khz;
	u32 freq_khz = freq_hz / 1000;

	if (!freq_hz) {
		printf("%s: the frequency can't be 0 Hz\n", __func__);
		return -1;
	}

	postdiv1 = DIV_ROUND_UP(VCO_MIN_HZ / 1000, freq_khz);
	if (postdiv1 > max_postdiv1) {
		postdiv2 = DIV_ROUND_UP(postdiv1, max_postdiv1);
		postdiv1 = DIV_ROUND_UP(postdiv1, postdiv2);
	}

	vco_khz = freq_khz * postdiv1 * postdiv2;

	if (vco_khz < (VCO_MIN_HZ / 1000) || vco_khz > (VCO_MAX_HZ / 1000) ||
	    postdiv2 > max_postdiv2) {
		printf("%s: Cannot find out a supported VCO for Freq (%uHz)\n",
		       __func__, freq_hz);
		return -1;
	}

	div->postdiv1 = postdiv1;
	div->postdiv2 = postdiv2;

	best_diff_khz = vco_khz;
	for (refdiv = 1; refdiv < max_refdiv && best_diff_khz; refdiv++) {
		fref_khz = ref_khz / refdiv;

		fbdiv = vco_khz / fref_khz;
		if ((fbdiv >= max_fbdiv) || (fbdiv <= min_fbdiv))
			continue;
		diff_khz = vco_khz - fbdiv * fref_khz;
		if (fbdiv + 1 < max_fbdiv && diff_khz > fref_khz / 2) {
			fbdiv++;
			diff_khz = fref_khz - diff_khz;
		}

		if (diff_khz >= best_diff_khz)
			continue;

		best_diff_khz = diff_khz;
		div->refdiv = refdiv;
		div->fbdiv = fbdiv;
	}

	if (best_diff_khz > 4 * (1000)) {
		printf("%s: Failed to match output frequency %u bestis %u Hz\n",
		       __func__, freq_hz,
		       best_diff_khz * 1000);
		return -1;
	}
	return 0;
}

static void rkclk_init(struct rk3128_cru *cru)
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
		     GPLL_MODE_MASK | APLL_MODE_MASK | CPLL_MODE_MASK,
		     GPLL_MODE_NORM << GPLL_MODE_SHIFT |
		     APLL_MODE_NORM << APLL_MODE_SHIFT |
		     CPLL_MODE_NORM << CPLL_MODE_SHIFT);

	/*fix NAND controller  working clock max to 150Mhz */
	rk_clrsetreg(&cru->cru_clksel_con[2],
		     NANDC_PLL_SEL_MASK | NANDC_CLK_DIV_MASK,
		     NANDC_PLL_SEL_GPLL << NANDC_PLL_SEL_SHIFT |
		     3 << NANDC_CLK_DIV_SHIFT);
}

/* Get pll rate by id */
static u32 rkclk_pll_get_rate(struct rk3128_cru *cru,
			      enum rk_clk_id clk_id)
{
	u32 refdiv, fbdiv, postdiv1, postdiv2;
	u32 con;
	int pll_id = rk_pll_id(clk_id);
	struct rk3128_pll *pll = &cru->pll[pll_id];
	static u8 clk_shift[CLK_COUNT] = {
		0xff, APLL_MODE_SHIFT, DPLL_MODE_SHIFT, CPLL_MODE_SHIFT,
		GPLL_MODE_SHIFT, 0xff
	};
	static u32 clk_mask[CLK_COUNT] = {
		0xff, APLL_MODE_MASK, DPLL_MODE_MASK, CPLL_MODE_MASK,
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
	case GPLL_MODE_DEEP:
	default:
		return 32768;
	}
}

static ulong rockchip_mmc_get_clk(struct rk3128_cru *cru, uint clk_general_rate,
				  int periph)
{
	uint src_rate;
	uint div, mux;
	u32 con;

	switch (periph) {
	case HCLK_EMMC:
	case SCLK_EMMC:
	case SCLK_EMMC_SAMPLE:
		con = readl(&cru->cru_clksel_con[12]);
		mux = (con & EMMC_PLL_MASK) >> EMMC_PLL_SHIFT;
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
	return DIV_TO_RATE(src_rate, div);
}

static ulong rockchip_mmc_set_clk(struct rk3128_cru *cru, uint clk_general_rate,
				  int periph, uint freq)
{
	int src_clk_div;
	int mux;

	debug("%s: clk_general_rate=%u\n", __func__, clk_general_rate);

	/* mmc clock defaulg div 2 internal, need provide double in cru */
	src_clk_div = DIV_ROUND_UP(clk_general_rate / 2, freq);

	if (src_clk_div > 128) {
		src_clk_div = DIV_ROUND_UP(OSC_HZ / 2, freq);
		mux = EMMC_SEL_24M;
	} else {
		mux = EMMC_SEL_GPLL;
	}

	switch (periph) {
	case HCLK_EMMC:
		rk_clrsetreg(&cru->cru_clksel_con[12],
			     EMMC_PLL_MASK | EMMC_DIV_MASK,
			     mux << EMMC_PLL_SHIFT |
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

static ulong rk3128_peri_get_pclk(struct rk3128_cru *cru, ulong clk_id)
{
	u32 div, con;

	switch (clk_id) {
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_PWM:
		con = readl(&cru->cru_clksel_con[10]);
		div = con >> 12 & 0x3;
		break;
	default:
		printf("do not support this peripheral bus\n");
		return -EINVAL;
	}

	return DIV_TO_RATE(PERI_ACLK_HZ, div);
}

static ulong rk3128_peri_set_pclk(struct rk3128_cru *cru, ulong clk_id, uint hz)
{
	int src_clk_div;

	src_clk_div = PERI_ACLK_HZ / hz;
	assert(src_clk_div - 1 < 4);

	switch (clk_id) {
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_PWM:
		rk_setreg(&cru->cru_clksel_con[10],
			  ((src_clk_div - 1) << 12));
		break;
	default:
		printf("do not support this peripheral bus\n");
		return -EINVAL;
	}

	return DIV_TO_RATE(PERI_ACLK_HZ, src_clk_div);
}

static ulong rk3128_saradc_get_clk(struct rk3128_cru *cru)
{
	u32 div, val;

	val = readl(&cru->cru_clksel_con[24]);
	div = bitfield_extract(val, SARADC_DIV_CON_SHIFT,
			       SARADC_DIV_CON_WIDTH);

	return DIV_TO_RATE(OSC_HZ, div);
}

static ulong rk3128_saradc_set_clk(struct rk3128_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(OSC_HZ, hz) - 1;
	assert(src_clk_div < 128);

	rk_clrsetreg(&cru->cru_clksel_con[24],
		     SARADC_DIV_CON_MASK,
		     src_clk_div << SARADC_DIV_CON_SHIFT);

	return rk3128_saradc_get_clk(cru);
}

static ulong rk3128_vop_set_clk(struct rk3128_cru *cru, ulong clk_id, uint hz)
{
	int src_clk_div;
	struct pll_div cpll_config = {0};

	src_clk_div = GPLL_HZ / hz;
	assert(src_clk_div - 1 < 31);

	switch (clk_id) {
	case ACLK_VIO0:
		rk_clrsetreg(&cru->cru_clksel_con[31],
			     VIO0_PLL_MASK | VIO0_DIV_MASK,
			     VIO0_SEL_GPLL << VIO0_PLL_SHIFT |
			     (src_clk_div - 1) << VIO0_DIV_SHIFT);
		break;
	case ACLK_VIO1:
		rk_clrsetreg(&cru->cru_clksel_con[31],
			     VIO1_PLL_MASK | VIO1_DIV_MASK,
			     VIO1_SEL_GPLL << VIO1_PLL_SHIFT |
			     (src_clk_div - 1) << VIO1_DIV_SHIFT);
		break;
	case DCLK_VOP:
		if (pll_para_config(hz, &cpll_config))
			return -1;
		rkclk_set_pll(cru, CLK_CODEC, &cpll_config);

		rk_clrsetreg(&cru->cru_clksel_con[27],
			     DCLK_VOP_SEL_MASK | DCLK_VOP_DIV_CON_MASK,
			     DCLK_VOP_PLL_SEL_CPLL << DCLK_VOP_SEL_SHIFT |
			     (1 - 1) << DCLK_VOP_DIV_CON_SHIFT);
		break;
	default:
		printf("do not support this vop freq\n");
		return -EINVAL;
	}

	return hz;
}

static ulong rk3128_vop_get_rate(struct rk3128_cru *cru, ulong clk_id)
{
	u32 div, con, parent;

	switch (clk_id) {
	case ACLK_VIO0:
		con = readl(&cru->cru_clksel_con[31]);
		div = con  & 0x1f;
		parent = GPLL_HZ;
		break;
	case ACLK_VIO1:
		con = readl(&cru->cru_clksel_con[31]);
		div = (con >> 8) & 0x1f;
		parent = GPLL_HZ;
		break;
	case DCLK_VOP:
		con = readl(&cru->cru_clksel_con[27]);
		div = (con >> 8) & 0xfff;
		parent = rkclk_pll_get_rate(cru, CLK_CODEC);
		break;
	default:
		return -ENOENT;
	}
	return DIV_TO_RATE(parent, div);
}

static ulong rk3128_clk_get_rate(struct clk *clk)
{
	struct rk3128_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case 0 ... 63:
		return rkclk_pll_get_rate(priv->cru, clk->id);
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_PWM:
		return rk3128_peri_get_pclk(priv->cru, clk->id);
	case SCLK_SARADC:
		return rk3128_saradc_get_clk(priv->cru);
	case DCLK_VOP:
	case ACLK_VIO0:
	case ACLK_VIO1:
		return rk3128_vop_get_rate(priv->cru, clk->id);
	default:
		return -ENOENT;
	}
}

static ulong rk3128_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3128_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate, gclk_rate;

	gclk_rate = rkclk_pll_get_rate(priv->cru, CLK_GENERAL);
	switch (clk->id) {
	case 0 ... 63:
		return 0;
	case DCLK_VOP:
	case ACLK_VIO0:
	case ACLK_VIO1:
		new_rate = rk3128_vop_set_clk(priv->cru,
					      clk->id, rate);
		break;
	case HCLK_EMMC:
		new_rate = rockchip_mmc_set_clk(priv->cru, gclk_rate,
						clk->id, rate);
		break;
	case PCLK_I2C0:
	case PCLK_I2C1:
	case PCLK_I2C2:
	case PCLK_I2C3:
	case PCLK_PWM:
		new_rate = rk3128_peri_set_pclk(priv->cru, clk->id, rate);
		break;
	case SCLK_SARADC:
		new_rate = rk3128_saradc_set_clk(priv->cru, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static struct clk_ops rk3128_clk_ops = {
	.get_rate	= rk3128_clk_get_rate,
	.set_rate	= rk3128_clk_set_rate,
};

static int rk3128_clk_of_to_plat(struct udevice *dev)
{
	struct rk3128_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rk3128_clk_probe(struct udevice *dev)
{
	struct rk3128_clk_priv *priv = dev_get_priv(dev);

	rkclk_init(priv->cru);

	return 0;
}

static int rk3128_clk_bind(struct udevice *dev)
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
		priv->glb_srst_fst_value = offsetof(struct rk3128_cru,
						    cru_glb_srst_fst_value);
		priv->glb_srst_snd_value = offsetof(struct rk3128_cru,
						    cru_glb_srst_snd_value);
		dev_set_priv(sys_child, priv);
	}

	return 0;
}

static const struct udevice_id rk3128_clk_ids[] = {
	{ .compatible = "rockchip,rk3128-cru" },
	{ .compatible = "rockchip,rk3126-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3128_cru) = {
	.name		= "clk_rk3128",
	.id		= UCLASS_CLK,
	.of_match	= rk3128_clk_ids,
	.priv_auto	= sizeof(struct rk3128_clk_priv),
	.of_to_plat = rk3128_clk_of_to_plat,
	.ops		= &rk3128_clk_ops,
	.bind		= rk3128_clk_bind,
	.probe		= rk3128_clk_probe,
};
