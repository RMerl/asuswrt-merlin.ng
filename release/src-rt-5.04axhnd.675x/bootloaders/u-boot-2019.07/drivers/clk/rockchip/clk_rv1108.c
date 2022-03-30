// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 */

#include <common.h>
#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rv1108.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rv1108-cru.h>

DECLARE_GLOBAL_DATA_PTR;

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
	.postdiv1 = _postdiv1, .postdiv2 = _postdiv2};\
	_Static_assert(((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ) *\
			 OSC_HZ / (_refdiv * _postdiv1 * _postdiv2) == hz,\
			 #hz "Hz cannot be hit with PLL "\
			 "divisors on line " __stringify(__LINE__));

static const struct pll_div apll_init_cfg = PLL_DIVISORS(APLL_HZ, 1, 3, 1);
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 2, 2, 1);

/* use integer mode */
static inline int rv1108_pll_id(enum rk_clk_id clk_id)
{
	int id = 0;

	switch (clk_id) {
	case CLK_ARM:
	case CLK_DDR:
		id = clk_id - 1;
		break;
	case CLK_GENERAL:
		id = 2;
		break;
	default:
		printf("invalid pll id:%d\n", clk_id);
		id = -1;
		break;
	}

	return id;
}

static int rkclk_set_pll(struct rv1108_cru *cru, enum rk_clk_id clk_id,
			 const struct pll_div *div)
{
	int pll_id = rv1108_pll_id(clk_id);
	struct rv1108_pll *pll = &cru->pll[pll_id];

	/* All PLLs have same VCO and output frequency range restrictions. */
	uint vco_hz = OSC_HZ / 1000 * div->fbdiv / div->refdiv * 1000;
	uint output_hz = vco_hz / div->postdiv1 / div->postdiv2;

	debug("PLL at %p: fb=%d, ref=%d, pst1=%d, pst2=%d, vco=%u Hz, output=%u Hz\n",
	      pll, div->fbdiv, div->refdiv, div->postdiv1,
	      div->postdiv2, vco_hz, output_hz);
	assert(vco_hz >= VCO_MIN_HZ && vco_hz <= VCO_MAX_HZ &&
	       output_hz >= OUTPUT_MIN_HZ && output_hz <= OUTPUT_MAX_HZ);

	/*
	 * When power on or changing PLL setting,
	 * we must force PLL into slow mode to ensure output stable clock.
	 */
	rk_clrsetreg(&pll->con3, WORK_MODE_MASK,
		     WORK_MODE_SLOW << WORK_MODE_SHIFT);

	/* use integer mode */
	rk_setreg(&pll->con3, 1 << DSMPD_SHIFT);
	/* Power down */
	rk_setreg(&pll->con3, 1 << GLOBAL_POWER_DOWN_SHIFT);

	rk_clrsetreg(&pll->con0, FBDIV_MASK, div->fbdiv << FBDIV_SHIFT);
	rk_clrsetreg(&pll->con1, POSTDIV1_MASK | POSTDIV2_MASK | REFDIV_MASK,
		     (div->postdiv1 << POSTDIV1_SHIFT |
		     div->postdiv2 << POSTDIV2_SHIFT |
		     div->refdiv << REFDIV_SHIFT));
	rk_clrsetreg(&pll->con2, FRACDIV_MASK,
		     (div->refdiv << REFDIV_SHIFT));

	/* Power Up */
	rk_clrreg(&pll->con3, 1 << GLOBAL_POWER_DOWN_SHIFT);

	/* waiting for pll lock */
	while (readl(&pll->con2) & (1 << LOCK_STA_SHIFT))
		udelay(1);

	/*
	 * set PLL into normal mode.
	 */
	rk_clrsetreg(&pll->con3, WORK_MODE_MASK,
		     WORK_MODE_NORMAL << WORK_MODE_SHIFT);

	return 0;
}

static uint32_t rkclk_pll_get_rate(struct rv1108_cru *cru,
				   enum rk_clk_id clk_id)
{
	uint32_t refdiv, fbdiv, postdiv1, postdiv2;
	uint32_t con0, con1, con3;
	int pll_id = rv1108_pll_id(clk_id);
	struct rv1108_pll *pll = &cru->pll[pll_id];
	uint32_t freq;

	con3 = readl(&pll->con3);

	if (con3 & WORK_MODE_MASK) {
		con0 = readl(&pll->con0);
		con1 = readl(&pll->con1);
		fbdiv = (con0 >> FBDIV_SHIFT) & FBDIV_MASK;
		postdiv1 = (con1 & POSTDIV1_MASK) >> POSTDIV1_SHIFT;
		postdiv2 = (con1 & POSTDIV2_MASK) >> POSTDIV2_SHIFT;
		refdiv = (con1 >> REFDIV_SHIFT) & REFDIV_MASK;
		freq = (24 * fbdiv / (refdiv * postdiv1 * postdiv2)) * 1000000;
	} else {
		freq = OSC_HZ;
	}

	return freq;
}

static int rv1108_mac_set_clk(struct rv1108_cru *cru, ulong rate)
{
	uint32_t con = readl(&cru->clksel_con[24]);
	ulong pll_rate;
	uint8_t div;

	if ((con >> MAC_PLL_SEL_SHIFT) & MAC_PLL_SEL_GPLL)
		pll_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);
	else
		pll_rate = rkclk_pll_get_rate(cru, CLK_ARM);

	/*default set 50MHZ for gmac*/
	if (!rate)
		rate = 50000000;

	div = DIV_ROUND_UP(pll_rate, rate) - 1;
	if (div <= 0x1f)
		rk_clrsetreg(&cru->clksel_con[24], MAC_CLK_DIV_MASK,
			     div << MAC_CLK_DIV_SHIFT);
	else
		debug("Unsupported div for gmac:%d\n", div);

	return DIV_TO_RATE(pll_rate, div);
}

static int rv1108_sfc_set_clk(struct rv1108_cru *cru, uint rate)
{
	u32 con = readl(&cru->clksel_con[27]);
	u32 pll_rate;
	u32 div;

	if ((con >> SFC_PLL_SEL_SHIFT) && SFC_PLL_SEL_GPLL)
		pll_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);
	else
		pll_rate = rkclk_pll_get_rate(cru, CLK_DDR);

	div = DIV_ROUND_UP(pll_rate, rate) - 1;
	if (div <= 0x3f)
		rk_clrsetreg(&cru->clksel_con[27], SFC_CLK_DIV_MASK,
			     div << SFC_CLK_DIV_SHIFT);
	else
		debug("Unsupported sfc clk rate:%d\n", rate);

	return DIV_TO_RATE(pll_rate, div);
}

static ulong rv1108_saradc_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;

	val = readl(&cru->clksel_con[22]);
	div = bitfield_extract(val, CLK_SARADC_DIV_CON_SHIFT,
			       CLK_SARADC_DIV_CON_WIDTH);

	return DIV_TO_RATE(OSC_HZ, div);
}

static ulong rv1108_saradc_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(OSC_HZ, hz) - 1;
	assert(src_clk_div < 128);

	rk_clrsetreg(&cru->clksel_con[22],
		     CLK_SARADC_DIV_CON_MASK,
		     src_clk_div << CLK_SARADC_DIV_CON_SHIFT);

	return rv1108_saradc_get_clk(cru);
}

static ulong rv1108_aclk_vio1_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;

	val = readl(&cru->clksel_con[28]);
	div = bitfield_extract(val, ACLK_VIO1_CLK_DIV_SHIFT,
			       CLK_VIO_DIV_CON_WIDTH);

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rv1108_aclk_vio1_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(GPLL_HZ, hz) - 1;
	assert(src_clk_div < 32);

	rk_clrsetreg(&cru->clksel_con[28],
		     ACLK_VIO1_CLK_DIV_MASK | ACLK_VIO1_PLL_SEL_MASK,
		     (src_clk_div << ACLK_VIO1_CLK_DIV_SHIFT) |
		     (VIO_PLL_SEL_GPLL << ACLK_VIO1_PLL_SEL_SHIFT));

	return rv1108_aclk_vio1_get_clk(cru);
}

static ulong rv1108_aclk_vio0_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;

	val = readl(&cru->clksel_con[28]);
	div = bitfield_extract(val, ACLK_VIO0_CLK_DIV_SHIFT,
			       CLK_VIO_DIV_CON_WIDTH);

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rv1108_aclk_vio0_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(GPLL_HZ, hz) - 1;
	assert(src_clk_div < 32);

	rk_clrsetreg(&cru->clksel_con[28],
		     ACLK_VIO0_CLK_DIV_MASK | ACLK_VIO0_PLL_SEL_MASK,
		     (src_clk_div << ACLK_VIO0_CLK_DIV_SHIFT) |
		     (VIO_PLL_SEL_GPLL << ACLK_VIO0_PLL_SEL_SHIFT));

	/*HCLK_VIO default div = 4*/
	rk_clrsetreg(&cru->clksel_con[29],
		     HCLK_VIO_CLK_DIV_MASK,
		     3 << HCLK_VIO_CLK_DIV_SHIFT);
	/*PCLK_VIO default div = 4*/
	rk_clrsetreg(&cru->clksel_con[29],
		     PCLK_VIO_CLK_DIV_MASK,
		     3 << PCLK_VIO_CLK_DIV_SHIFT);

	return rv1108_aclk_vio0_get_clk(cru);
}

static ulong rv1108_dclk_vop_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;

	val = readl(&cru->clksel_con[32]);
	div = bitfield_extract(val, DCLK_VOP_CLK_DIV_SHIFT,
			       DCLK_VOP_DIV_CON_WIDTH);

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rv1108_dclk_vop_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(GPLL_HZ, hz) - 1;
	assert(src_clk_div < 64);

	rk_clrsetreg(&cru->clksel_con[32],
		     DCLK_VOP_CLK_DIV_MASK | DCLK_VOP_PLL_SEL_MASK |
		     DCLK_VOP_SEL_SHIFT,
		     (src_clk_div << DCLK_VOP_CLK_DIV_SHIFT) |
		     (DCLK_VOP_PLL_SEL_GPLL << DCLK_VOP_PLL_SEL_SHIFT) |
		     (DCLK_VOP_SEL_PLL << DCLK_VOP_SEL_SHIFT));

	return rv1108_dclk_vop_get_clk(cru);
}

static ulong rv1108_aclk_bus_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	val = readl(&cru->clksel_con[2]);
	div = bitfield_extract(val, ACLK_BUS_DIV_CON_SHIFT,
			       ACLK_BUS_DIV_CON_WIDTH);

	return DIV_TO_RATE(parent_rate, div);
}

static ulong rv1108_aclk_bus_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	src_clk_div = DIV_ROUND_UP(parent_rate, hz) - 1;
	assert(src_clk_div < 32);

	rk_clrsetreg(&cru->clksel_con[2],
		     ACLK_BUS_DIV_CON_MASK | ACLK_BUS_PLL_SEL_MASK,
		     (src_clk_div << ACLK_BUS_DIV_CON_SHIFT) |
		     (ACLK_BUS_PLL_SEL_GPLL << ACLK_BUS_PLL_SEL_SHIFT));

	return rv1108_aclk_bus_get_clk(cru);
}

static ulong rv1108_aclk_peri_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	val = readl(&cru->clksel_con[23]);
	div = bitfield_extract(val, ACLK_PERI_DIV_CON_SHIFT,
			       PERI_DIV_CON_WIDTH);

	return DIV_TO_RATE(parent_rate, div);
}

static ulong rv1108_hclk_peri_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	val = readl(&cru->clksel_con[23]);
	div = bitfield_extract(val, HCLK_PERI_DIV_CON_SHIFT,
			       PERI_DIV_CON_WIDTH);

	return DIV_TO_RATE(parent_rate, div);
}

static ulong rv1108_pclk_peri_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	val = readl(&cru->clksel_con[23]);
	div = bitfield_extract(val, PCLK_PERI_DIV_CON_SHIFT,
			       PERI_DIV_CON_WIDTH);

	return DIV_TO_RATE(parent_rate, div);
}

static ulong rv1108_aclk_peri_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	src_clk_div = DIV_ROUND_UP(parent_rate, hz) - 1;
	assert(src_clk_div < 32);

	rk_clrsetreg(&cru->clksel_con[23],
		     ACLK_PERI_DIV_CON_MASK | ACLK_PERI_PLL_SEL_MASK,
		     (src_clk_div << ACLK_PERI_DIV_CON_SHIFT) |
		     (ACLK_PERI_PLL_SEL_GPLL << ACLK_PERI_PLL_SEL_SHIFT));

	return rv1108_aclk_peri_get_clk(cru);
}

static ulong rv1108_hclk_peri_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	src_clk_div = DIV_ROUND_UP(parent_rate, hz) - 1;
	assert(src_clk_div < 32);

	rk_clrsetreg(&cru->clksel_con[23],
		     HCLK_PERI_DIV_CON_MASK,
		     (src_clk_div << HCLK_PERI_DIV_CON_SHIFT));

	return rv1108_hclk_peri_get_clk(cru);
}

static ulong rv1108_pclk_peri_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;
	ulong parent_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);

	src_clk_div = DIV_ROUND_UP(parent_rate, hz) - 1;
	assert(src_clk_div < 32);

	rk_clrsetreg(&cru->clksel_con[23],
		     PCLK_PERI_DIV_CON_MASK,
		     (src_clk_div << PCLK_PERI_DIV_CON_SHIFT));

	return rv1108_pclk_peri_get_clk(cru);
}

static ulong rv1108_i2c_get_clk(struct rv1108_cru *cru, ulong clk_id)
{
	u32 div, con;

	switch (clk_id) {
	case SCLK_I2C0_PMU:
		con = readl(&cru->clksel_con[19]);
		div = bitfield_extract(con, CLK_I2C0_DIV_CON_SHIFT,
				       I2C_DIV_CON_WIDTH);
		break;
	case SCLK_I2C1:
		con = readl(&cru->clksel_con[19]);
		div = bitfield_extract(con, CLK_I2C1_DIV_CON_SHIFT,
				       I2C_DIV_CON_WIDTH);
		break;
	case SCLK_I2C2:
		con = readl(&cru->clksel_con[20]);
		div = bitfield_extract(con, CLK_I2C2_DIV_CON_SHIFT,
				       I2C_DIV_CON_WIDTH);
		break;
	case SCLK_I2C3:
		con = readl(&cru->clksel_con[20]);
		div = bitfield_extract(con, CLK_I2C3_DIV_CON_SHIFT,
				       I2C_DIV_CON_WIDTH);
		break;
	default:
		printf("do not support this i2c bus\n");
		return -EINVAL;
	}

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rv1108_i2c_set_clk(struct rv1108_cru *cru, ulong clk_id, uint hz)
{
	int src_clk_div;

	/* i2c0,4,8 src clock from ppll, i2c1,2,3,5,6,7 src clock from gpll*/
	src_clk_div = GPLL_HZ / hz;
	assert(src_clk_div - 1 <= 127);

	switch (clk_id) {
	case SCLK_I2C0_PMU:
		rk_clrsetreg(&cru->clksel_con[19],
			     CLK_I2C0_DIV_CON_MASK | CLK_I2C1_PLL_SEL_MASK,
			     (src_clk_div << CLK_I2C0_DIV_CON_SHIFT) |
			     (CLK_I2C1_PLL_SEL_GPLL << CLK_I2C1_PLL_SEL_SHIFT));
		break;
	case SCLK_I2C1:
		rk_clrsetreg(&cru->clksel_con[19],
			     CLK_I2C1_DIV_CON_MASK | CLK_I2C1_PLL_SEL_MASK,
			     (src_clk_div << CLK_I2C1_DIV_CON_SHIFT) |
			     (CLK_I2C1_PLL_SEL_GPLL << CLK_I2C1_PLL_SEL_SHIFT));
		break;
	case SCLK_I2C2:
		rk_clrsetreg(&cru->clksel_con[20],
			     CLK_I2C2_DIV_CON_MASK | CLK_I2C3_PLL_SEL_MASK,
			     (src_clk_div << CLK_I2C2_DIV_CON_SHIFT) |
			     (CLK_I2C3_PLL_SEL_GPLL << CLK_I2C3_PLL_SEL_SHIFT));
		break;
	case SCLK_I2C3:
		rk_clrsetreg(&cru->clksel_con[20],
			     CLK_I2C3_DIV_CON_MASK | CLK_I2C3_PLL_SEL_MASK,
			     (src_clk_div << CLK_I2C3_DIV_CON_SHIFT) |
			     (CLK_I2C3_PLL_SEL_GPLL << CLK_I2C3_PLL_SEL_SHIFT));
		break;
	default:
		printf("do not support this i2c bus\n");
		return -EINVAL;
	}

	return rv1108_i2c_get_clk(cru, clk_id);
}

static ulong rv1108_mmc_get_clk(struct rv1108_cru *cru)
{
	u32 div, con;
	ulong mmc_clk;

	con = readl(&cru->clksel_con[26]);
	div = bitfield_extract(con, EMMC_CLK_DIV_SHIFT, 8);

	con = readl(&cru->clksel_con[25]);

	if ((con & EMMC_PLL_SEL_MASK) >> EMMC_PLL_SEL_SHIFT == EMMC_PLL_SEL_OSC)
		mmc_clk = DIV_TO_RATE(OSC_HZ, div) / 2;
	else
		mmc_clk = DIV_TO_RATE(GPLL_HZ, div) / 2;

	debug("%s div %d get_clk %ld\n", __func__, div, mmc_clk);
	return mmc_clk;
}

static ulong rv1108_mmc_set_clk(struct rv1108_cru *cru, ulong rate)
{
	int div;
	u32 pll_rate;

	div = DIV_ROUND_UP(rkclk_pll_get_rate(cru, CLK_GENERAL), rate);

	if (div < 127) {
		debug("%s source gpll\n", __func__);
		rk_clrsetreg(&cru->clksel_con[25], EMMC_PLL_SEL_MASK,
			    (EMMC_PLL_SEL_GPLL << EMMC_PLL_SEL_SHIFT));
		pll_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);
	} else {
		debug("%s source 24m\n", __func__);
		rk_clrsetreg(&cru->clksel_con[25], EMMC_PLL_SEL_MASK,
			    (EMMC_PLL_SEL_OSC << EMMC_PLL_SEL_SHIFT));
		pll_rate = OSC_HZ;
	}

	div = DIV_ROUND_UP(pll_rate / 2, rate);
	rk_clrsetreg(&cru->clksel_con[26], EMMC_CLK_DIV_MASK,
		    ((div - 1) << EMMC_CLK_DIV_SHIFT));

	debug("%s set_rate %ld div %d\n", __func__,  rate, div);

	return DIV_TO_RATE(pll_rate, div);
}

static ulong rv1108_clk_get_rate(struct clk *clk)
{
	struct rv1108_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case 0 ... 63:
		return rkclk_pll_get_rate(priv->cru, clk->id);
	case SCLK_SARADC:
		return rv1108_saradc_get_clk(priv->cru);
	case ACLK_VIO0:
		return rv1108_aclk_vio0_get_clk(priv->cru);
	case ACLK_VIO1:
		return rv1108_aclk_vio1_get_clk(priv->cru);
	case DCLK_VOP:
		return rv1108_dclk_vop_get_clk(priv->cru);
	case ACLK_PRE:
		return rv1108_aclk_bus_get_clk(priv->cru);
	case ACLK_PERI:
		return rv1108_aclk_peri_get_clk(priv->cru);
	case HCLK_PERI:
		return rv1108_hclk_peri_get_clk(priv->cru);
	case PCLK_PERI:
		return rv1108_pclk_peri_get_clk(priv->cru);
	case SCLK_I2C0_PMU:
	case SCLK_I2C1:
	case SCLK_I2C2:
	case SCLK_I2C3:
		return rv1108_i2c_get_clk(priv->cru, clk->id);
	case HCLK_EMMC:
	case SCLK_EMMC:
	case SCLK_EMMC_SAMPLE:
		return rv1108_mmc_get_clk(priv->cru);
	default:
		return -ENOENT;
	}
}

static ulong rv1108_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rv1108_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate;

	switch (clk->id) {
	case SCLK_MAC:
		new_rate = rv1108_mac_set_clk(priv->cru, rate);
		break;
	case SCLK_SFC:
		new_rate = rv1108_sfc_set_clk(priv->cru, rate);
		break;
	case SCLK_SARADC:
		new_rate = rv1108_saradc_set_clk(priv->cru, rate);
		break;
	case ACLK_VIO0:
		new_rate = rv1108_aclk_vio0_set_clk(priv->cru, rate);
		break;
	case ACLK_VIO1:
		new_rate = rv1108_aclk_vio1_set_clk(priv->cru, rate);
		break;
	case DCLK_VOP:
		new_rate = rv1108_dclk_vop_set_clk(priv->cru, rate);
		break;
	case ACLK_PRE:
		new_rate = rv1108_aclk_bus_set_clk(priv->cru, rate);
		break;
	case ACLK_PERI:
		new_rate = rv1108_aclk_peri_set_clk(priv->cru, rate);
		break;
	case HCLK_PERI:
		new_rate = rv1108_hclk_peri_set_clk(priv->cru, rate);
		break;
	case PCLK_PERI:
		new_rate = rv1108_pclk_peri_set_clk(priv->cru, rate);
		break;
	case SCLK_I2C0_PMU:
	case SCLK_I2C1:
	case SCLK_I2C2:
	case SCLK_I2C3:
		new_rate = rv1108_i2c_set_clk(priv->cru, clk->id, rate);
		break;
	case HCLK_EMMC:
	case SCLK_EMMC:
		new_rate = rv1108_mmc_set_clk(priv->cru, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static const struct clk_ops rv1108_clk_ops = {
	.get_rate	= rv1108_clk_get_rate,
	.set_rate	= rv1108_clk_set_rate,
};

static void rkclk_init(struct rv1108_cru *cru)
{
	unsigned int apll, dpll, gpll;
	unsigned int aclk_bus, aclk_peri, hclk_peri, pclk_peri;

	aclk_bus = rv1108_aclk_bus_set_clk(cru, ACLK_BUS_HZ / 2);
	aclk_peri = rv1108_aclk_peri_set_clk(cru, ACLK_PERI_HZ / 2);
	hclk_peri = rv1108_hclk_peri_set_clk(cru, HCLK_PERI_HZ / 2);
	pclk_peri = rv1108_pclk_peri_set_clk(cru, PCLK_PERI_HZ / 2);
	rv1108_aclk_vio0_set_clk(cru, 297000000);
	rv1108_aclk_vio1_set_clk(cru, 297000000);

	/* configure apll */
	rkclk_set_pll(cru, CLK_ARM, &apll_init_cfg);
	rkclk_set_pll(cru, CLK_GENERAL, &gpll_init_cfg);
	aclk_bus = rv1108_aclk_bus_set_clk(cru, ACLK_BUS_HZ);
	aclk_peri = rv1108_aclk_peri_set_clk(cru, ACLK_PERI_HZ);
	hclk_peri = rv1108_hclk_peri_set_clk(cru, HCLK_PERI_HZ);
	pclk_peri = rv1108_pclk_peri_set_clk(cru, PCLK_PERI_HZ);

	apll = rkclk_pll_get_rate(cru, CLK_ARM);
	dpll = rkclk_pll_get_rate(cru, CLK_DDR);
	gpll = rkclk_pll_get_rate(cru, CLK_GENERAL);

	rk_clrsetreg(&cru->clksel_con[0], CORE_CLK_DIV_MASK,
		     0 << MAC_CLK_DIV_SHIFT);

	printf("APLL: %d DPLL:%d GPLL:%d\n", apll, dpll, gpll);
	printf("ACLK_BUS: %d ACLK_PERI:%d HCLK_PERI:%d PCLK_PERI:%d\n",
	       aclk_bus, aclk_peri, hclk_peri, pclk_peri);
}

static int rv1108_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rv1108_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rv1108_clk_probe(struct udevice *dev)
{
	struct rv1108_clk_priv *priv = dev_get_priv(dev);

	rkclk_init(priv->cru);

	return 0;
}

static int rv1108_clk_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child, *sf_child;
	struct sysreset_reg *priv;
	struct softreset_reg *sf_priv;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rv1108_cru,
						    glb_srst_fst_val);
		priv->glb_srst_snd_value = offsetof(struct rv1108_cru,
						    glb_srst_snd_val);
		sys_child->priv = priv;
	}

#if CONFIG_IS_ENABLED(CONFIG_RESET_ROCKCHIP)
	ret = offsetof(struct rk3368_cru, softrst_con[0]);
	ret = rockchip_reset_bind(dev, ret, 13);
	if (ret)
		debug("Warning: software reset driver bind faile\n");
#endif
	ret = device_bind_driver_to_node(dev, "rockchip_reset", "reset",
					 dev_ofnode(dev), &sf_child);
	if (ret) {
		debug("Warning: No rockchip reset driver: ret=%d\n", ret);
	} else {
		sf_priv = malloc(sizeof(struct softreset_reg));
		sf_priv->sf_reset_offset = offsetof(struct rv1108_cru,
						    softrst_con[0]);
		sf_priv->sf_reset_num = 13;
		sf_child->priv = sf_priv;
	}

	return 0;
}

static const struct udevice_id rv1108_clk_ids[] = {
	{ .compatible = "rockchip,rv1108-cru" },
	{ }
};

U_BOOT_DRIVER(clk_rv1108) = {
	.name		= "clk_rv1108",
	.id		= UCLASS_CLK,
	.of_match	= rv1108_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rv1108_clk_priv),
	.ops		= &rv1108_clk_ops,
	.bind		= rv1108_clk_bind,
	.ofdata_to_platdata	= rv1108_clk_ofdata_to_platdata,
	.probe		= rv1108_clk_probe,
};
