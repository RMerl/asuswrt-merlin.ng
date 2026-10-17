// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Fuzhou Rockchip Electronics Co., Ltd
 * Author: Elaine Zhang <zhangqing@rock-chips.com>
 */

#include <common.h>
#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <scmi_protocols.h>
#include <syscon.h>
#include <asm/arch-rockchip/cru_rk3588.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rockchip,rk3588-cru.h>

DECLARE_GLOBAL_DATA_PTR;

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

static struct rockchip_pll_rate_table rk3588_pll_rates[] = {
	/* _mhz, _p, _m, _s, _k */
	RK3588_PLL_RATE(1500000000, 2, 250, 1, 0),
	RK3588_PLL_RATE(1200000000, 2, 200, 1, 0),
	RK3588_PLL_RATE(1188000000, 2, 198, 1, 0),
	RK3588_PLL_RATE(1100000000, 3, 550, 2, 0),
	RK3588_PLL_RATE(1008000000, 2, 336, 2, 0),
	RK3588_PLL_RATE(1000000000, 3, 500, 2, 0),
	RK3588_PLL_RATE(900000000, 2, 300, 2, 0),
	RK3588_PLL_RATE(850000000, 3, 425, 2, 0),
	RK3588_PLL_RATE(816000000, 2, 272, 2, 0),
	RK3588_PLL_RATE(786432000, 2, 262, 2, 9437),
	RK3588_PLL_RATE(786000000, 1, 131, 2, 0),
	RK3588_PLL_RATE(742500000, 4, 495, 2, 0),
	RK3588_PLL_RATE(722534400, 8, 963, 2, 24850),
	RK3588_PLL_RATE(600000000, 2, 200, 2, 0),
	RK3588_PLL_RATE(594000000, 2, 198, 2, 0),
	RK3588_PLL_RATE(200000000, 3, 400, 4, 0),
	RK3588_PLL_RATE(100000000, 3, 400, 5, 0),
	{ /* sentinel */ },
};

static struct rockchip_pll_clock rk3588_pll_clks[] = {
	[B0PLL] = PLL(pll_rk3588, PLL_B0PLL, RK3588_B0_PLL_CON(0),
		      RK3588_B0_PLL_MODE_CON, 0, 15, 0,
		      rk3588_pll_rates),
	[B1PLL] = PLL(pll_rk3588, PLL_B1PLL, RK3588_B1_PLL_CON(8),
		      RK3588_B1_PLL_MODE_CON, 0, 15, 0,
		      rk3588_pll_rates),
	[LPLL] = PLL(pll_rk3588, PLL_LPLL, RK3588_LPLL_CON(16),
		     RK3588_LPLL_MODE_CON, 0, 15, 0, rk3588_pll_rates),
	[V0PLL] = PLL(pll_rk3588, PLL_V0PLL, RK3588_PLL_CON(88),
		      RK3588_MODE_CON0, 4, 15, 0, rk3588_pll_rates),
	[AUPLL] = PLL(pll_rk3588, PLL_AUPLL, RK3588_PLL_CON(96),
		      RK3588_MODE_CON0, 6, 15, 0, rk3588_pll_rates),
	[CPLL] = PLL(pll_rk3588, PLL_CPLL, RK3588_PLL_CON(104),
		     RK3588_MODE_CON0, 8, 15, 0, rk3588_pll_rates),
	[GPLL] = PLL(pll_rk3588, PLL_GPLL, RK3588_PLL_CON(112),
		     RK3588_MODE_CON0, 2, 15, 0, rk3588_pll_rates),
	[NPLL] = PLL(pll_rk3588, PLL_NPLL, RK3588_PLL_CON(120),
		     RK3588_MODE_CON0, 0, 15, 0, rk3588_pll_rates),
	[PPLL] = PLL(pll_rk3588, PLL_PPLL, RK3588_PMU_PLL_CON(128),
		     RK3588_MODE_CON0, 10, 15, 0, rk3588_pll_rates),
};

#ifndef CONFIG_SPL_BUILD
/*
 *
 * rational_best_approximation(31415, 10000,
 *		(1 << 8) - 1, (1 << 5) - 1, &n, &d);
 *
 * you may look at given_numerator as a fixed point number,
 * with the fractional part size described in given_denominator.
 *
 * for theoretical background, see:
 * http://en.wikipedia.org/wiki/Continued_fraction
 */
static void rational_best_approximation(unsigned long given_numerator,
					unsigned long given_denominator,
					unsigned long max_numerator,
					unsigned long max_denominator,
					unsigned long *best_numerator,
					unsigned long *best_denominator)
{
	unsigned long n, d, n0, d0, n1, d1;

	n = given_numerator;
	d = given_denominator;
	n0 = 0;
	d1 = 0;
	n1 = 1;
	d0 = 1;
	for (;;) {
		unsigned long t, a;

		if (n1 > max_numerator || d1 > max_denominator) {
			n1 = n0;
			d1 = d0;
			break;
		}
		if (d == 0)
			break;
		t = d;
		a = n / d;
		d = n % d;
		n = t;
		t = n0 + a * n1;
		n0 = n1;
		n1 = t;
		t = d0 + a * d1;
		d0 = d1;
		d1 = t;
	}
	*best_numerator = n1;
	*best_denominator = d1;
}
#endif

static ulong rk3588_center_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 con, sel, rate;

	switch (clk_id) {
	case ACLK_CENTER_ROOT:
		con = readl(&cru->clksel_con[165]);
		sel = (con & ACLK_CENTER_ROOT_SEL_MASK) >>
		      ACLK_CENTER_ROOT_SEL_SHIFT;
		if (sel == ACLK_CENTER_ROOT_SEL_700M)
			rate = 702 * MHz;
		else if (sel == ACLK_CENTER_ROOT_SEL_400M)
			rate = 396 * MHz;
		else if (sel == ACLK_CENTER_ROOT_SEL_200M)
			rate = 200 * MHz;
		else
			rate = OSC_HZ;
		break;
	case ACLK_CENTER_LOW_ROOT:
		con = readl(&cru->clksel_con[165]);
		sel = (con & ACLK_CENTER_LOW_ROOT_SEL_MASK) >>
		      ACLK_CENTER_LOW_ROOT_SEL_SHIFT;
		if (sel == ACLK_CENTER_LOW_ROOT_SEL_500M)
			rate = 500 * MHz;
		else if (sel == ACLK_CENTER_LOW_ROOT_SEL_250M)
			rate = 250 * MHz;
		else if (sel == ACLK_CENTER_LOW_ROOT_SEL_100M)
			rate = 100 * MHz;
		else
			rate = OSC_HZ;
		break;
	case HCLK_CENTER_ROOT:
		con = readl(&cru->clksel_con[165]);
		sel = (con & HCLK_CENTER_ROOT_SEL_MASK) >>
		      HCLK_CENTER_ROOT_SEL_SHIFT;
		if (sel == HCLK_CENTER_ROOT_SEL_400M)
			rate = 396 * MHz;
		else if (sel == HCLK_CENTER_ROOT_SEL_200M)
			rate = 200 * MHz;
		else if (sel == HCLK_CENTER_ROOT_SEL_100M)
			rate = 100 * MHz;
		else
			rate = OSC_HZ;
		break;
	case PCLK_CENTER_ROOT:
		con = readl(&cru->clksel_con[165]);
		sel = (con & PCLK_CENTER_ROOT_SEL_MASK) >>
		      PCLK_CENTER_ROOT_SEL_SHIFT;
		if (sel == PCLK_CENTER_ROOT_SEL_200M)
			rate = 200 * MHz;
		else if (sel == PCLK_CENTER_ROOT_SEL_100M)
			rate = 100 * MHz;
		else if (sel == PCLK_CENTER_ROOT_SEL_50M)
			rate = 50 * MHz;
		else
			rate = OSC_HZ;
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3588_center_set_clk(struct rk3588_clk_priv *priv,
				   ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk;

	switch (clk_id) {
	case ACLK_CENTER_ROOT:
		if (rate >= 700 * MHz)
			src_clk = ACLK_CENTER_ROOT_SEL_700M;
		else if (rate >= 396 * MHz)
			src_clk = ACLK_CENTER_ROOT_SEL_400M;
		else if (rate >= 200 * MHz)
			src_clk = ACLK_CENTER_ROOT_SEL_200M;
		else
			src_clk = ACLK_CENTER_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[165],
			     ACLK_CENTER_ROOT_SEL_MASK,
			     src_clk << ACLK_CENTER_ROOT_SEL_SHIFT);
		break;
	case ACLK_CENTER_LOW_ROOT:
		if (rate >= 500 * MHz)
			src_clk = ACLK_CENTER_LOW_ROOT_SEL_500M;
		else if (rate >= 250 * MHz)
			src_clk = ACLK_CENTER_LOW_ROOT_SEL_250M;
		else if (rate >= 99 * MHz)
			src_clk = ACLK_CENTER_LOW_ROOT_SEL_100M;
		else
			src_clk = ACLK_CENTER_LOW_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[165],
			     ACLK_CENTER_LOW_ROOT_SEL_MASK,
			     src_clk << ACLK_CENTER_LOW_ROOT_SEL_SHIFT);
		break;
	case HCLK_CENTER_ROOT:
		if (rate >= 396 * MHz)
			src_clk = HCLK_CENTER_ROOT_SEL_400M;
		else if (rate >= 198 * MHz)
			src_clk = HCLK_CENTER_ROOT_SEL_200M;
		else if (rate >= 99 * MHz)
			src_clk = HCLK_CENTER_ROOT_SEL_100M;
		else
			src_clk = HCLK_CENTER_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[165],
			     HCLK_CENTER_ROOT_SEL_MASK,
			     src_clk << HCLK_CENTER_ROOT_SEL_SHIFT);
		break;
	case PCLK_CENTER_ROOT:
		if (rate >= 198 * MHz)
			src_clk = PCLK_CENTER_ROOT_SEL_200M;
		else if (rate >= 99 * MHz)
			src_clk = PCLK_CENTER_ROOT_SEL_100M;
		else if (rate >= 50 * MHz)
			src_clk = PCLK_CENTER_ROOT_SEL_50M;
		else
			src_clk = PCLK_CENTER_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[165],
			     PCLK_CENTER_ROOT_SEL_MASK,
			     src_clk << PCLK_CENTER_ROOT_SEL_SHIFT);
		break;
	default:
		printf("do not support this center freq\n");
		return -EINVAL;
	}

	return rk3588_center_get_clk(priv, clk_id);
}

static ulong rk3588_top_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 con, sel, div, rate, prate;

	switch (clk_id) {
	case ACLK_TOP_ROOT:
		con = readl(&cru->clksel_con[8]);
		div = (con & ACLK_TOP_ROOT_DIV_MASK) >>
		      ACLK_TOP_ROOT_DIV_SHIFT;
		sel = (con & ACLK_TOP_ROOT_SRC_SEL_MASK) >>
		      ACLK_TOP_ROOT_SRC_SEL_SHIFT;
		if (sel == ACLK_TOP_ROOT_SRC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case ACLK_LOW_TOP_ROOT:
		con = readl(&cru->clksel_con[8]);
		div = (con & ACLK_LOW_TOP_ROOT_DIV_MASK) >>
		      ACLK_LOW_TOP_ROOT_DIV_SHIFT;
		sel = (con & ACLK_LOW_TOP_ROOT_SRC_SEL_MASK) >>
		      ACLK_LOW_TOP_ROOT_SRC_SEL_SHIFT;
		if (sel == ACLK_LOW_TOP_ROOT_SRC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case PCLK_TOP_ROOT:
		con = readl(&cru->clksel_con[8]);
		sel = (con & PCLK_TOP_ROOT_SEL_MASK) >> PCLK_TOP_ROOT_SEL_SHIFT;
		if (sel == PCLK_TOP_ROOT_SEL_100M)
			rate = 100 * MHz;
		else if (sel == PCLK_TOP_ROOT_SEL_50M)
			rate = 50 * MHz;
		else
			rate = OSC_HZ;
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3588_top_set_clk(struct rk3588_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk, src_clk_div;

	switch (clk_id) {
	case ACLK_TOP_ROOT:
		if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_TOP_ROOT_SRC_SEL_CPLL;
			src_clk_div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_TOP_ROOT_SRC_SEL_GPLL;
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		assert(src_clk_div - 1 <= 31);
		rk_clrsetreg(&cru->clksel_con[8],
			     ACLK_TOP_ROOT_DIV_MASK |
			     ACLK_TOP_ROOT_SRC_SEL_MASK,
			     (src_clk <<
			      ACLK_TOP_ROOT_SRC_SEL_SHIFT) |
			     (src_clk_div - 1) << ACLK_TOP_ROOT_DIV_SHIFT);
		break;
	case ACLK_LOW_TOP_ROOT:
		src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
		assert(src_clk_div - 1 <= 31);
		rk_clrsetreg(&cru->clksel_con[8],
			     ACLK_LOW_TOP_ROOT_DIV_MASK |
			     ACLK_LOW_TOP_ROOT_SRC_SEL_MASK,
			     (ACLK_LOW_TOP_ROOT_SRC_SEL_GPLL <<
			      ACLK_LOW_TOP_ROOT_SRC_SEL_SHIFT) |
			     (src_clk_div - 1) << ACLK_LOW_TOP_ROOT_DIV_SHIFT);
		break;
	case PCLK_TOP_ROOT:
		if (rate == 100 * MHz)
			src_clk = PCLK_TOP_ROOT_SEL_100M;
		else if (rate == 50 * MHz)
			src_clk = PCLK_TOP_ROOT_SEL_50M;
		else
			src_clk = PCLK_TOP_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[8],
			     PCLK_TOP_ROOT_SEL_MASK,
			     src_clk << PCLK_TOP_ROOT_SEL_SHIFT);
		break;
	default:
		printf("do not support this top freq\n");
		return -EINVAL;
	}

	return rk3588_top_get_clk(priv, clk_id);
}

static ulong rk3588_i2c_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 sel, con;
	ulong rate;

	switch (clk_id) {
	case CLK_I2C0:
		con = readl(&cru->pmuclksel_con[3]);
		sel = (con & CLK_I2C0_SEL_MASK) >> CLK_I2C0_SEL_SHIFT;
		break;
	case CLK_I2C1:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C1_SEL_MASK) >> CLK_I2C1_SEL_SHIFT;
		break;
	case CLK_I2C2:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C2_SEL_MASK) >> CLK_I2C2_SEL_SHIFT;
		break;
	case CLK_I2C3:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C3_SEL_MASK) >> CLK_I2C3_SEL_SHIFT;
		break;
	case CLK_I2C4:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C4_SEL_MASK) >> CLK_I2C4_SEL_SHIFT;
		break;
	case CLK_I2C5:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C5_SEL_MASK) >> CLK_I2C5_SEL_SHIFT;
		break;
	case CLK_I2C6:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C6_SEL_MASK) >> CLK_I2C6_SEL_SHIFT;
		break;
	case CLK_I2C7:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C7_SEL_MASK) >> CLK_I2C7_SEL_SHIFT;
		break;
	case CLK_I2C8:
		con = readl(&cru->clksel_con[38]);
		sel = (con & CLK_I2C8_SEL_MASK) >> CLK_I2C8_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}
	if (sel == CLK_I2C_SEL_200M)
		rate = 200 * MHz;
	else
		rate = 100 * MHz;

	return rate;
}

static ulong rk3588_i2c_set_clk(struct rk3588_clk_priv *priv, ulong clk_id,
				ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 198 * MHz)
		src_clk = CLK_I2C_SEL_200M;
	else
		src_clk = CLK_I2C_SEL_100M;

	switch (clk_id) {
	case CLK_I2C0:
		rk_clrsetreg(&cru->pmuclksel_con[3], CLK_I2C0_SEL_MASK,
			     src_clk << CLK_I2C0_SEL_SHIFT);
		break;
	case CLK_I2C1:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C1_SEL_MASK,
			     src_clk << CLK_I2C1_SEL_SHIFT);
		break;
	case CLK_I2C2:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C2_SEL_MASK,
			     src_clk << CLK_I2C2_SEL_SHIFT);
		break;
	case CLK_I2C3:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C3_SEL_MASK,
			     src_clk << CLK_I2C3_SEL_SHIFT);
		break;
	case CLK_I2C4:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C4_SEL_MASK,
			     src_clk << CLK_I2C4_SEL_SHIFT);
		break;
	case CLK_I2C5:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C5_SEL_MASK,
			     src_clk << CLK_I2C5_SEL_SHIFT);
		break;
	case CLK_I2C6:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C6_SEL_MASK,
			     src_clk << CLK_I2C6_SEL_SHIFT);
		break;
	case CLK_I2C7:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C7_SEL_MASK,
			     src_clk << CLK_I2C7_SEL_SHIFT);
		break;
	case CLK_I2C8:
		rk_clrsetreg(&cru->clksel_con[38], CLK_I2C8_SEL_MASK,
			     src_clk << CLK_I2C8_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_i2c_get_clk(priv, clk_id);
}

static ulong rk3588_spi_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 sel, con;

	con = readl(&cru->clksel_con[59]);

	switch (clk_id) {
	case CLK_SPI0:
		sel = (con & CLK_SPI0_SEL_MASK) >> CLK_SPI0_SEL_SHIFT;
		break;
	case CLK_SPI1:
		sel = (con & CLK_SPI1_SEL_MASK) >> CLK_SPI1_SEL_SHIFT;
		break;
	case CLK_SPI2:
		sel = (con & CLK_SPI2_SEL_MASK) >> CLK_SPI2_SEL_SHIFT;
		break;
	case CLK_SPI3:
		sel = (con & CLK_SPI3_SEL_MASK) >> CLK_SPI3_SEL_SHIFT;
		break;
	case CLK_SPI4:
		sel = (con & CLK_SPI4_SEL_MASK) >> CLK_SPI4_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	switch (sel) {
	case CLK_SPI_SEL_200M:
		return 200 * MHz;
	case CLK_SPI_SEL_150M:
		return 150 * MHz;
	case CLK_SPI_SEL_24M:
		return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rk3588_spi_set_clk(struct rk3588_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 198 * MHz)
		src_clk = CLK_SPI_SEL_200M;
	else if (rate >= 140 * MHz)
		src_clk = CLK_SPI_SEL_150M;
	else
		src_clk = CLK_SPI_SEL_24M;

	switch (clk_id) {
	case CLK_SPI0:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_SPI0_SEL_MASK,
			     src_clk << CLK_SPI0_SEL_SHIFT);
		break;
	case CLK_SPI1:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_SPI1_SEL_MASK,
			     src_clk << CLK_SPI1_SEL_SHIFT);
		break;
	case CLK_SPI2:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_SPI2_SEL_MASK,
			     src_clk << CLK_SPI2_SEL_SHIFT);
		break;
	case CLK_SPI3:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_SPI3_SEL_MASK,
			     src_clk << CLK_SPI3_SEL_SHIFT);
		break;
	case CLK_SPI4:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_SPI4_SEL_MASK,
			     src_clk << CLK_SPI4_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_spi_get_clk(priv, clk_id);
}

static ulong rk3588_pwm_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 sel, con;

	switch (clk_id) {
	case CLK_PWM1:
		con = readl(&cru->clksel_con[59]);
		sel = (con & CLK_PWM1_SEL_MASK) >> CLK_PWM1_SEL_SHIFT;
		break;
	case CLK_PWM2:
		con = readl(&cru->clksel_con[59]);
		sel = (con & CLK_PWM2_SEL_MASK) >> CLK_PWM2_SEL_SHIFT;
		break;
	case CLK_PWM3:
		con = readl(&cru->clksel_con[60]);
		sel = (con & CLK_PWM3_SEL_MASK) >> CLK_PWM3_SEL_SHIFT;
		break;
	case CLK_PMU1PWM:
		con = readl(&cru->pmuclksel_con[2]);
		sel = (con & CLK_PMU1PWM_SEL_MASK) >> CLK_PMU1PWM_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	switch (sel) {
	case CLK_PWM_SEL_100M:
		return 100 * MHz;
	case CLK_PWM_SEL_50M:
		return 50 * MHz;
	case CLK_PWM_SEL_24M:
		return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rk3588_pwm_set_clk(struct rk3588_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 99 * MHz)
		src_clk = CLK_PWM_SEL_100M;
	else if (rate >= 50 * MHz)
		src_clk = CLK_PWM_SEL_50M;
	else
		src_clk = CLK_PWM_SEL_24M;

	switch (clk_id) {
	case CLK_PWM1:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_PWM1_SEL_MASK,
			     src_clk << CLK_PWM1_SEL_SHIFT);
		break;
	case CLK_PWM2:
		rk_clrsetreg(&cru->clksel_con[59],
			     CLK_PWM2_SEL_MASK,
			     src_clk << CLK_PWM2_SEL_SHIFT);
		break;
	case CLK_PWM3:
		rk_clrsetreg(&cru->clksel_con[60],
			     CLK_PWM3_SEL_MASK,
			     src_clk << CLK_PWM3_SEL_SHIFT);
		break;
	case CLK_PMU1PWM:
		rk_clrsetreg(&cru->pmuclksel_con[2],
			     CLK_PMU1PWM_SEL_MASK,
			     src_clk << CLK_PMU1PWM_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_pwm_get_clk(priv, clk_id);
}

static ulong rk3588_adc_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 div, sel, con, prate;

	switch (clk_id) {
	case CLK_SARADC:
		con = readl(&cru->clksel_con[40]);
		div = (con & CLK_SARADC_DIV_MASK) >> CLK_SARADC_DIV_SHIFT;
		sel = (con & CLK_SARADC_SEL_MASK) >>
		      CLK_SARADC_SEL_SHIFT;
		if (sel == CLK_SARADC_SEL_24M)
			prate = OSC_HZ;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case CLK_TSADC:
		con = readl(&cru->clksel_con[41]);
		div = (con & CLK_TSADC_DIV_MASK) >>
		      CLK_TSADC_DIV_SHIFT;
		sel = (con & CLK_TSADC_SEL_MASK) >>
		      CLK_TSADC_SEL_SHIFT;
		if (sel == CLK_TSADC_SEL_24M)
			prate = OSC_HZ;
		else
			prate = 100 * MHz;
		return DIV_TO_RATE(prate, div);
	default:
		return -ENOENT;
	}
}

static ulong rk3588_adc_set_clk(struct rk3588_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk_div;

	switch (clk_id) {
	case CLK_SARADC:
		if (!(OSC_HZ % rate)) {
			src_clk_div = DIV_ROUND_UP(OSC_HZ, rate);
			assert(src_clk_div - 1 <= 255);
			rk_clrsetreg(&cru->clksel_con[40],
				     CLK_SARADC_SEL_MASK |
				     CLK_SARADC_DIV_MASK,
				     (CLK_SARADC_SEL_24M <<
				      CLK_SARADC_SEL_SHIFT) |
				     (src_clk_div - 1) <<
				     CLK_SARADC_DIV_SHIFT);
		} else {
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
			assert(src_clk_div - 1 <= 255);
			rk_clrsetreg(&cru->clksel_con[40],
				     CLK_SARADC_SEL_MASK |
				     CLK_SARADC_DIV_MASK,
				     (CLK_SARADC_SEL_GPLL <<
				      CLK_SARADC_SEL_SHIFT) |
				     (src_clk_div - 1) <<
				     CLK_SARADC_DIV_SHIFT);
		}
		break;
	case CLK_TSADC:
		if (!(OSC_HZ % rate)) {
			src_clk_div = DIV_ROUND_UP(OSC_HZ, rate);
			assert(src_clk_div - 1 <= 255);
			rk_clrsetreg(&cru->clksel_con[41],
				     CLK_TSADC_SEL_MASK |
				     CLK_TSADC_DIV_MASK,
				     (CLK_TSADC_SEL_24M <<
				      CLK_TSADC_SEL_SHIFT) |
				     (src_clk_div - 1) <<
				     CLK_TSADC_DIV_SHIFT);
		} else {
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
			assert(src_clk_div - 1 <= 7);
			rk_clrsetreg(&cru->clksel_con[41],
				     CLK_TSADC_SEL_MASK |
				     CLK_TSADC_DIV_MASK,
				     (CLK_TSADC_SEL_GPLL <<
				      CLK_TSADC_SEL_SHIFT) |
				     (src_clk_div - 1) <<
				     CLK_TSADC_DIV_SHIFT);
		}
		break;
	default:
		return -ENOENT;
	}
	return rk3588_adc_get_clk(priv, clk_id);
}

static ulong rk3588_mmc_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 sel, con, div, prate;

	switch (clk_id) {
	case CCLK_SRC_SDIO:
		con = readl(&cru->clksel_con[172]);
		div = (con & CCLK_SDIO_SRC_DIV_MASK) >> CCLK_SDIO_SRC_DIV_SHIFT;
		sel = (con & CCLK_SDIO_SRC_SEL_MASK) >>
		      CCLK_SDIO_SRC_SEL_SHIFT;
		if (sel == CCLK_SDIO_SRC_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == CCLK_SDIO_SRC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case CCLK_EMMC:
		con = readl(&cru->clksel_con[77]);
		div = (con & CCLK_EMMC_DIV_MASK) >> CCLK_EMMC_DIV_SHIFT;
		sel = (con & CCLK_EMMC_SEL_MASK) >>
		      CCLK_EMMC_SEL_SHIFT;
		if (sel == CCLK_EMMC_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == CCLK_EMMC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case BCLK_EMMC:
		con = readl(&cru->clksel_con[78]);
		div = (con & BCLK_EMMC_DIV_MASK) >> BCLK_EMMC_DIV_SHIFT;
		sel = (con & BCLK_EMMC_SEL_MASK) >>
		      BCLK_EMMC_SEL_SHIFT;
		if (sel == CCLK_EMMC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case SCLK_SFC:
		con = readl(&cru->clksel_con[78]);
		div = (con & SCLK_SFC_DIV_MASK) >> SCLK_SFC_DIV_SHIFT;
		sel = (con & SCLK_SFC_SEL_MASK) >>
		      SCLK_SFC_SEL_SHIFT;
		if (sel == SCLK_SFC_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == SCLK_SFC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case DCLK_DECOM:
		con = readl(&cru->clksel_con[62]);
		div = (con & DCLK_DECOM_DIV_MASK) >> DCLK_DECOM_DIV_SHIFT;
		sel = (con & DCLK_DECOM_SEL_MASK) >>
		      DCLK_DECOM_SEL_SHIFT;
		if (sel == DCLK_DECOM_SEL_SPLL)
			prate = 702 * MHz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	default:
		return -ENOENT;
	}
}

static ulong rk3588_mmc_set_clk(struct rk3588_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk, div;

	switch (clk_id) {
	case CCLK_SRC_SDIO:
	case CCLK_EMMC:
	case SCLK_SFC:
		if (!(OSC_HZ % rate)) {
			src_clk = SCLK_SFC_SEL_24M;
			div = DIV_ROUND_UP(OSC_HZ, rate);
		} else if (!(priv->cpll_hz % rate)) {
			src_clk = SCLK_SFC_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = SCLK_SFC_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		break;
	case BCLK_EMMC:
		if (!(priv->cpll_hz % rate)) {
			src_clk = CCLK_EMMC_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = CCLK_EMMC_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		break;
	case DCLK_DECOM:
		if (!(702 * MHz % rate)) {
			src_clk = DCLK_DECOM_SEL_SPLL;
			div = DIV_ROUND_UP(702 * MHz, rate);
		} else {
			src_clk = DCLK_DECOM_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		break;
	default:
		return -ENOENT;
	}

	switch (clk_id) {
	case CCLK_SRC_SDIO:
		rk_clrsetreg(&cru->clksel_con[172],
			     CCLK_SDIO_SRC_SEL_MASK |
			     CCLK_SDIO_SRC_DIV_MASK,
			     (src_clk << CCLK_SDIO_SRC_SEL_SHIFT) |
			     (div - 1) << CCLK_SDIO_SRC_DIV_SHIFT);
		break;
	case CCLK_EMMC:
		rk_clrsetreg(&cru->clksel_con[77],
			     CCLK_EMMC_SEL_MASK |
			     CCLK_EMMC_DIV_MASK,
			     (src_clk << CCLK_EMMC_SEL_SHIFT) |
			     (div - 1) << CCLK_EMMC_DIV_SHIFT);
		break;
	case BCLK_EMMC:
		rk_clrsetreg(&cru->clksel_con[78],
			     BCLK_EMMC_DIV_MASK |
			     BCLK_EMMC_SEL_MASK,
			     (src_clk << BCLK_EMMC_SEL_SHIFT) |
			     (div - 1) << BCLK_EMMC_DIV_SHIFT);
		break;
	case SCLK_SFC:
		rk_clrsetreg(&cru->clksel_con[78],
			     SCLK_SFC_DIV_MASK |
			     SCLK_SFC_SEL_MASK,
			     (src_clk << SCLK_SFC_SEL_SHIFT) |
			     (div - 1) << SCLK_SFC_DIV_SHIFT);
		break;
	case DCLK_DECOM:
		rk_clrsetreg(&cru->clksel_con[62],
			     DCLK_DECOM_DIV_MASK |
			     DCLK_DECOM_SEL_MASK,
			     (src_clk << DCLK_DECOM_SEL_SHIFT) |
			     (div - 1) << DCLK_DECOM_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_mmc_get_clk(priv, clk_id);
}

#ifndef CONFIG_SPL_BUILD
static ulong rk3588_aux16m_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 div, con, parent;

	parent = priv->gpll_hz;
	con = readl(&cru->clksel_con[117]);

	switch (clk_id) {
	case CLK_AUX16M_0:
		div = (con & CLK_AUX16MHZ_0_DIV_MASK) >> CLK_AUX16MHZ_0_DIV_SHIFT;
		return DIV_TO_RATE(parent, div);
	case CLK_AUX16M_1:
		div = (con & CLK_AUX16MHZ_1_DIV_MASK) >> CLK_AUX16MHZ_1_DIV_SHIFT;
		return DIV_TO_RATE(parent, div);
	default:
		return -ENOENT;
	}
}

static ulong rk3588_aux16m_set_clk(struct rk3588_clk_priv *priv,
				   ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	u32 div;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	div = DIV_ROUND_UP(priv->gpll_hz, rate);

	switch (clk_id) {
	case CLK_AUX16M_0:
		rk_clrsetreg(&cru->clksel_con[117], CLK_AUX16MHZ_0_DIV_MASK,
			     (div - 1) << CLK_AUX16MHZ_0_DIV_SHIFT);
		break;
	case CLK_AUX16M_1:
		rk_clrsetreg(&cru->clksel_con[117], CLK_AUX16MHZ_1_DIV_MASK,
			     (div - 1) << CLK_AUX16MHZ_1_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_aux16m_get_clk(priv, clk_id);
}

static ulong rk3588_aclk_vop_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 div, sel, con, parent;

	switch (clk_id) {
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
		con = readl(&cru->clksel_con[110]);
		div = (con & ACLK_VOP_ROOT_DIV_MASK) >> ACLK_VOP_ROOT_DIV_SHIFT;
		sel = (con & ACLK_VOP_ROOT_SEL_MASK) >> ACLK_VOP_ROOT_SEL_SHIFT;
		if (sel == ACLK_VOP_ROOT_SEL_GPLL)
			parent = priv->gpll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_CPLL)
			parent = priv->cpll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_AUPLL)
			parent = priv->aupll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_NPLL)
			parent = priv->npll_hz;
		else
			parent = 702 * MHz;
		return DIV_TO_RATE(parent, div);
	case ACLK_VOP_LOW_ROOT:
		con = readl(&cru->clksel_con[110]);
		sel = (con & ACLK_VOP_LOW_ROOT_SEL_MASK) >>
		      ACLK_VOP_LOW_ROOT_SEL_SHIFT;
		if (sel == ACLK_VOP_LOW_ROOT_SEL_400M)
			return 396 * MHz;
		else if (sel == ACLK_VOP_LOW_ROOT_SEL_200M)
			return 200 * MHz;
		else if (sel == ACLK_VOP_LOW_ROOT_SEL_100M)
			return 100 * MHz;
		else
			return OSC_HZ;
	case HCLK_VOP_ROOT:
		con = readl(&cru->clksel_con[110]);
		sel = (con & HCLK_VOP_ROOT_SEL_MASK) >> HCLK_VOP_ROOT_SEL_SHIFT;
		if (sel == HCLK_VOP_ROOT_SEL_200M)
			return 200 * MHz;
		else if (sel == HCLK_VOP_ROOT_SEL_100M)
			return 100 * MHz;
		else if (sel == HCLK_VOP_ROOT_SEL_50M)
			return 50 * MHz;
		else
			return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rk3588_aclk_vop_set_clk(struct rk3588_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int src_clk, div;

	switch (clk_id) {
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
		if (rate >= 850 * MHz) {
			src_clk = ACLK_VOP_ROOT_SEL_NPLL;
			div = 1;
		} else if (rate >= 750 * MHz) {
			src_clk = ACLK_VOP_ROOT_SEL_CPLL;
			div = 2;
		} else if (rate >= 700 * MHz) {
			src_clk = ACLK_VOP_ROOT_SEL_SPLL;
			div = 1;
		} else if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_VOP_ROOT_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_VOP_ROOT_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		rk_clrsetreg(&cru->clksel_con[110],
			     ACLK_VOP_ROOT_DIV_MASK |
			     ACLK_VOP_ROOT_SEL_MASK,
			     (src_clk << ACLK_VOP_ROOT_SEL_SHIFT) |
			     (div - 1) << ACLK_VOP_ROOT_DIV_SHIFT);
		break;
	case ACLK_VOP_LOW_ROOT:
		if (rate == 400 * MHz || rate == 396 * MHz)
			src_clk = ACLK_VOP_LOW_ROOT_SEL_400M;
		else if (rate == 200 * MHz)
			src_clk = ACLK_VOP_LOW_ROOT_SEL_200M;
		else if (rate == 100 * MHz)
			src_clk = ACLK_VOP_LOW_ROOT_SEL_100M;
		else
			src_clk = ACLK_VOP_LOW_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[110],
			     ACLK_VOP_LOW_ROOT_SEL_MASK,
			     src_clk << ACLK_VOP_LOW_ROOT_SEL_SHIFT);
		break;
	case HCLK_VOP_ROOT:
		if (rate == 200 * MHz)
			src_clk = HCLK_VOP_ROOT_SEL_200M;
		else if (rate == 100 * MHz)
			src_clk = HCLK_VOP_ROOT_SEL_100M;
		else if (rate == 50 * MHz)
			src_clk = HCLK_VOP_ROOT_SEL_50M;
		else
			src_clk = HCLK_VOP_ROOT_SEL_24M;
		rk_clrsetreg(&cru->clksel_con[110],
			     HCLK_VOP_ROOT_SEL_MASK,
			     src_clk << HCLK_VOP_ROOT_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_aclk_vop_get_clk(priv, clk_id);
}

static ulong rk3588_dclk_vop_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 div, sel, con, parent;

	switch (clk_id) {
	case DCLK_VOP0:
	case DCLK_VOP0_SRC:
		con = readl(&cru->clksel_con[111]);
		div = (con & DCLK0_VOP_SRC_DIV_MASK) >> DCLK0_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VOP1:
	case DCLK_VOP1_SRC:
		con = readl(&cru->clksel_con[111]);
		div = (con & DCLK1_VOP_SRC_DIV_MASK) >> DCLK1_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK1_VOP_SRC_SEL_MASK) >> DCLK1_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VOP2:
	case DCLK_VOP2_SRC:
		con = readl(&cru->clksel_con[112]);
		div = (con & DCLK2_VOP_SRC_DIV_MASK) >> DCLK2_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK2_VOP_SRC_SEL_MASK) >> DCLK2_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VOP3:
		con = readl(&cru->clksel_con[113]);
		div = (con & DCLK3_VOP_SRC_DIV_MASK) >> DCLK3_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK3_VOP_SRC_SEL_MASK) >> DCLK3_VOP_SRC_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == DCLK_VOP_SRC_SEL_AUPLL)
		parent = priv->aupll_hz;
	else if (sel == DCLK_VOP_SRC_SEL_V0PLL)
		parent = rockchip_pll_get_rate(&rk3588_pll_clks[V0PLL],
					       priv->cru, V0PLL);
	else if (sel == DCLK_VOP_SRC_SEL_GPLL)
		parent = priv->gpll_hz;
	else if (sel == DCLK_VOP_SRC_SEL_CPLL)
		parent = priv->cpll_hz;
	else
		return -ENOENT;

	return DIV_TO_RATE(parent, div);
}

#define RK3588_VOP_PLL_LIMIT_FREQ 600000000

static ulong rk3588_dclk_vop_set_clk(struct rk3588_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	ulong pll_rate, now, best_rate = 0;
	u32 i, conid, con, sel, div, best_div = 0, best_sel = 0;
	u32 mask, div_shift, sel_shift;

	switch (clk_id) {
	case DCLK_VOP0:
	case DCLK_VOP0_SRC:
		conid = 111;
		con = readl(&cru->clksel_con[111]);
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		mask = DCLK0_VOP_SRC_SEL_MASK | DCLK0_VOP_SRC_DIV_MASK;
		div_shift = DCLK0_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VOP1:
	case DCLK_VOP1_SRC:
		conid = 111;
		con = readl(&cru->clksel_con[111]);
		sel = (con & DCLK1_VOP_SRC_SEL_MASK) >> DCLK1_VOP_SRC_SEL_SHIFT;
		mask = DCLK1_VOP_SRC_SEL_MASK | DCLK1_VOP_SRC_DIV_MASK;
		div_shift = DCLK1_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK1_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VOP2:
	case DCLK_VOP2_SRC:
		conid = 112;
		con = readl(&cru->clksel_con[112]);
		sel = (con & DCLK2_VOP_SRC_SEL_MASK) >> DCLK2_VOP_SRC_SEL_SHIFT;
		mask = DCLK2_VOP_SRC_SEL_MASK | DCLK2_VOP_SRC_DIV_MASK;
		div_shift = DCLK2_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK2_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VOP3:
		conid = 113;
		con = readl(&cru->clksel_con[113]);
		sel = (con & DCLK3_VOP_SRC_SEL_MASK) >> DCLK3_VOP_SRC_SEL_SHIFT;
		mask = DCLK3_VOP_SRC_SEL_MASK | DCLK3_VOP_SRC_DIV_MASK;
		div_shift = DCLK3_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK3_VOP_SRC_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == DCLK_VOP_SRC_SEL_V0PLL) {
		pll_rate = rockchip_pll_get_rate(&rk3588_pll_clks[V0PLL],
						 priv->cru, V0PLL);
		if (pll_rate >= RK3588_VOP_PLL_LIMIT_FREQ && pll_rate % rate == 0) {
			div = DIV_ROUND_UP(pll_rate, rate);
			rk_clrsetreg(&cru->clksel_con[conid],
				     mask,
				     DCLK_VOP_SRC_SEL_V0PLL << sel_shift |
				     ((div - 1) << div_shift));
		} else {
			div = DIV_ROUND_UP(RK3588_VOP_PLL_LIMIT_FREQ, rate);
			rk_clrsetreg(&cru->clksel_con[conid],
				     mask,
				     DCLK_VOP_SRC_SEL_V0PLL << sel_shift |
				     ((div - 1) << div_shift));
			rockchip_pll_set_rate(&rk3588_pll_clks[V0PLL],
					      priv->cru, V0PLL, div * rate);
		}
	} else {
		for (i = 0; i <= DCLK_VOP_SRC_SEL_AUPLL; i++) {
			switch (i) {
			case DCLK_VOP_SRC_SEL_GPLL:
				pll_rate = priv->gpll_hz;
				break;
			case DCLK_VOP_SRC_SEL_CPLL:
				pll_rate = priv->cpll_hz;
				break;
			case DCLK_VOP_SRC_SEL_AUPLL:
				pll_rate = priv->aupll_hz;
				break;
			case DCLK_VOP_SRC_SEL_V0PLL:
				pll_rate = 0;
				break;
			default:
				printf("do not support this vop pll sel\n");
				return -EINVAL;
			}

			div = DIV_ROUND_UP(pll_rate, rate);
			if (div > 255)
				continue;
			now = pll_rate / div;
			if (abs(rate - now) < abs(rate - best_rate)) {
				best_rate = now;
				best_div = div;
				best_sel = i;
			}
			debug("p_rate=%lu, best_rate=%lu, div=%u, sel=%u\n",
			      pll_rate, best_rate, best_div, best_sel);
		}

		if (best_rate) {
			rk_clrsetreg(&cru->clksel_con[conid],
				     mask,
				     best_sel << sel_shift |
				     (best_div - 1) << div_shift);
		} else {
			printf("do not support this vop freq %lu\n", rate);
			return -EINVAL;
		}
	}
	return rk3588_dclk_vop_get_clk(priv, clk_id);
}

static ulong rk3588_gmac_get_clk(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 con, div;

	switch (clk_id) {
	case CLK_GMAC0_PTP_REF:
		con = readl(&cru->clksel_con[81]);
		div = (con & CLK_GMAC0_PTP_DIV_MASK) >> CLK_GMAC0_PTP_DIV_SHIFT;
		return DIV_TO_RATE(priv->cpll_hz, div);
	case CLK_GMAC1_PTP_REF:
		con = readl(&cru->clksel_con[81]);
		div = (con & CLK_GMAC1_PTP_DIV_MASK) >> CLK_GMAC1_PTP_DIV_SHIFT;
		return DIV_TO_RATE(priv->cpll_hz, div);
	case CLK_GMAC_125M:
		con = readl(&cru->clksel_con[83]);
		div = (con & CLK_GMAC_125M_DIV_MASK) >> CLK_GMAC_125M_DIV_SHIFT;
		return DIV_TO_RATE(priv->cpll_hz, div);
	case CLK_GMAC_50M:
		con = readl(&cru->clksel_con[84]);
		div = (con & CLK_GMAC_50M_DIV_MASK) >> CLK_GMAC_50M_DIV_SHIFT;
		return DIV_TO_RATE(priv->cpll_hz, div);
	default:
		return -ENOENT;
	}
}

static ulong rk3588_gmac_set_clk(struct rk3588_clk_priv *priv,
				 ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	int div;

	div = DIV_ROUND_UP(priv->cpll_hz, rate);

	switch (clk_id) {
	case CLK_GMAC0_PTP_REF:
		rk_clrsetreg(&cru->clksel_con[81],
			     CLK_GMAC0_PTP_DIV_MASK | CLK_GMAC0_PTP_SEL_MASK,
			     CLK_GMAC0_PTP_SEL_CPLL << CLK_GMAC0_PTP_SEL_SHIFT |
			     (div - 1) << CLK_GMAC0_PTP_DIV_SHIFT);
		break;
	case CLK_GMAC1_PTP_REF:
		rk_clrsetreg(&cru->clksel_con[81],
			     CLK_GMAC1_PTP_DIV_MASK | CLK_GMAC1_PTP_SEL_MASK,
			     CLK_GMAC1_PTP_SEL_CPLL << CLK_GMAC1_PTP_SEL_SHIFT |
			     (div - 1) << CLK_GMAC1_PTP_DIV_SHIFT);
		break;

	case CLK_GMAC_125M:
		rk_clrsetreg(&cru->clksel_con[83],
			     CLK_GMAC_125M_DIV_MASK | CLK_GMAC_125M_SEL_MASK,
			     CLK_GMAC_125M_SEL_CPLL << CLK_GMAC_125M_SEL_SHIFT |
			     (div - 1) << CLK_GMAC_125M_DIV_SHIFT);
		break;
	case CLK_GMAC_50M:
		rk_clrsetreg(&cru->clksel_con[84],
			     CLK_GMAC_50M_DIV_MASK | CLK_GMAC_50M_SEL_MASK,
			     CLK_GMAC_50M_SEL_CPLL << CLK_GMAC_50M_SEL_SHIFT |
			     (div - 1) << CLK_GMAC_50M_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_gmac_get_clk(priv, clk_id);
}

static ulong rk3588_uart_get_rate(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 reg, con, fracdiv, div, src, p_src, p_rate;
	unsigned long m, n;

	switch (clk_id) {
	case SCLK_UART1:
		reg = 41;
		break;
	case SCLK_UART2:
		reg = 43;
		break;
	case SCLK_UART3:
		reg = 45;
		break;
	case SCLK_UART4:
		reg = 47;
		break;
	case SCLK_UART5:
		reg = 49;
		break;
	case SCLK_UART6:
		reg = 51;
		break;
	case SCLK_UART7:
		reg = 53;
		break;
	case SCLK_UART8:
		reg = 55;
		break;
	case SCLK_UART9:
		reg = 57;
		break;
	default:
		return -ENOENT;
	}
	con = readl(&cru->clksel_con[reg + 2]);
	src = (con & CLK_UART_SEL_MASK) >> CLK_UART_SEL_SHIFT;
	con = readl(&cru->clksel_con[reg]);
	div = (con & CLK_UART_SRC_DIV_MASK) >> CLK_UART_SRC_DIV_SHIFT;
	p_src = (con & CLK_UART_SRC_SEL_MASK) >> CLK_UART_SRC_SEL_SHIFT;
	if (p_src == CLK_UART_SRC_SEL_GPLL)
		p_rate = priv->gpll_hz;
	else
		p_rate = priv->cpll_hz;

	if (src == CLK_UART_SEL_SRC) {
		return DIV_TO_RATE(p_rate, div);
	} else if (src == CLK_UART_SEL_FRAC) {
		fracdiv = readl(&cru->clksel_con[reg + 1]);
		n = fracdiv & CLK_UART_FRAC_NUMERATOR_MASK;
		n >>= CLK_UART_FRAC_NUMERATOR_SHIFT;
		m = fracdiv & CLK_UART_FRAC_DENOMINATOR_MASK;
		m >>= CLK_UART_FRAC_DENOMINATOR_SHIFT;
		return DIV_TO_RATE(p_rate, div) * n / m;
	} else {
		return OSC_HZ;
	}
}

static ulong rk3588_uart_set_rate(struct rk3588_clk_priv *priv,
				  ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	u32 reg, clk_src, uart_src, div;
	unsigned long m = 0, n = 0, val;

	if (priv->gpll_hz % rate == 0) {
		clk_src = CLK_UART_SRC_SEL_GPLL;
		uart_src = CLK_UART_SEL_SRC;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	} else if (priv->cpll_hz % rate == 0) {
		clk_src = CLK_UART_SRC_SEL_CPLL;
		uart_src = CLK_UART_SEL_SRC;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	} else if (rate == OSC_HZ) {
		clk_src = CLK_UART_SRC_SEL_GPLL;
		uart_src = CLK_UART_SEL_XIN24M;
		div = 2;
	} else {
		clk_src = CLK_UART_SRC_SEL_GPLL;
		uart_src = CLK_UART_SEL_FRAC;
		div = 2;
		rational_best_approximation(rate, priv->gpll_hz / div,
					    GENMASK(16 - 1, 0),
					    GENMASK(16 - 1, 0),
					    &m, &n);
	}

	switch (clk_id) {
	case SCLK_UART1:
		reg = 41;
		break;
	case SCLK_UART2:
		reg = 43;
		break;
	case SCLK_UART3:
		reg = 45;
		break;
	case SCLK_UART4:
		reg = 47;
		break;
	case SCLK_UART5:
		reg = 49;
		break;
	case SCLK_UART6:
		reg = 51;
		break;
	case SCLK_UART7:
		reg = 53;
		break;
	case SCLK_UART8:
		reg = 55;
		break;
	case SCLK_UART9:
		reg = 57;
		break;
	default:
		return -ENOENT;
	}
	rk_clrsetreg(&cru->clksel_con[reg],
		     CLK_UART_SRC_SEL_MASK |
		     CLK_UART_SRC_DIV_MASK,
		     (clk_src << CLK_UART_SRC_SEL_SHIFT) |
		     ((div - 1) << CLK_UART_SRC_DIV_SHIFT));
	rk_clrsetreg(&cru->clksel_con[reg + 2],
		     CLK_UART_SEL_MASK,
		     (uart_src << CLK_UART_SEL_SHIFT));
	if (m && n) {
		val = m << CLK_UART_FRAC_NUMERATOR_SHIFT | n;
		writel(val, &cru->clksel_con[reg + 1]);
	}

	return rk3588_uart_get_rate(priv, clk_id);
}

static ulong rk3588_pciephy_get_rate(struct rk3588_clk_priv *priv, ulong clk_id)
{
	struct rk3588_cru *cru = priv->cru;
	u32 con, div, src;

	switch (clk_id) {
	case CLK_REF_PIPE_PHY0:
		con = readl(&cru->clksel_con[177]);
		src = (con & CLK_PCIE_PHY0_REF_SEL_MASK) >> CLK_PCIE_PHY0_REF_SEL_SHIFT;
		con = readl(&cru->clksel_con[176]);
		div = (con & CLK_PCIE_PHY0_PLL_DIV_MASK) >> CLK_PCIE_PHY0_PLL_DIV_SHIFT;
		break;
	case CLK_REF_PIPE_PHY1:
		con = readl(&cru->clksel_con[177]);
		src = (con & CLK_PCIE_PHY1_REF_SEL_MASK) >> CLK_PCIE_PHY1_REF_SEL_SHIFT;
		con = readl(&cru->clksel_con[176]);
		div = (con & CLK_PCIE_PHY1_PLL_DIV_MASK) >> CLK_PCIE_PHY1_PLL_DIV_SHIFT;
		break;
	case CLK_REF_PIPE_PHY2:
		con = readl(&cru->clksel_con[177]);
		src = (con & CLK_PCIE_PHY2_REF_SEL_MASK) >> CLK_PCIE_PHY2_REF_SEL_SHIFT;
		div = (con & CLK_PCIE_PHY2_PLL_DIV_MASK) >> CLK_PCIE_PHY2_PLL_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (src == CLK_PCIE_PHY_REF_SEL_PPLL)
		return DIV_TO_RATE(priv->ppll_hz, div);
	else
		return OSC_HZ;
}

static ulong rk3588_pciephy_set_rate(struct rk3588_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3588_cru *cru = priv->cru;
	u32 clk_src, div;

	if (rate == OSC_HZ) {
		clk_src = CLK_PCIE_PHY_REF_SEL_24M;
		div = 1;
	} else {
		clk_src = CLK_PCIE_PHY_REF_SEL_PPLL;
		div = DIV_ROUND_UP(priv->ppll_hz, rate);
	}

	switch (clk_id) {
	case CLK_REF_PIPE_PHY0:
		rk_clrsetreg(&cru->clksel_con[177], CLK_PCIE_PHY0_REF_SEL_MASK,
			     (clk_src << CLK_PCIE_PHY0_REF_SEL_SHIFT));
		rk_clrsetreg(&cru->clksel_con[176], CLK_PCIE_PHY0_PLL_DIV_MASK,
			     ((div - 1) << CLK_PCIE_PHY0_PLL_DIV_SHIFT));
		break;
	case CLK_REF_PIPE_PHY1:
		rk_clrsetreg(&cru->clksel_con[177], CLK_PCIE_PHY1_REF_SEL_MASK,
			     (clk_src << CLK_PCIE_PHY1_REF_SEL_SHIFT));
		rk_clrsetreg(&cru->clksel_con[176], CLK_PCIE_PHY1_PLL_DIV_MASK,
			     ((div - 1) << CLK_PCIE_PHY1_PLL_DIV_SHIFT));
		break;
	case CLK_REF_PIPE_PHY2:
		rk_clrsetreg(&cru->clksel_con[177], CLK_PCIE_PHY2_REF_SEL_MASK |
			     CLK_PCIE_PHY2_PLL_DIV_MASK,
			     (clk_src << CLK_PCIE_PHY2_REF_SEL_SHIFT) |
			     ((div - 1) << CLK_PCIE_PHY2_PLL_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}

	return rk3588_pciephy_get_rate(priv, clk_id);
}
#endif

static ulong rk3588_clk_get_rate(struct clk *clk)
{
	struct rk3588_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	if (!priv->ppll_hz) {
		priv->ppll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[PPLL],
						      priv->cru, PPLL);
	}

	switch (clk->id) {
	case PLL_LPLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[LPLL], priv->cru,
					     LPLL);
		break;
	case PLL_B0PLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[B0PLL], priv->cru,
					     B0PLL);
		break;
	case PLL_B1PLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[B1PLL], priv->cru,
					     B1PLL);
		break;
	case PLL_GPLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[GPLL], priv->cru,
					     GPLL);
		break;
	case PLL_CPLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[CPLL], priv->cru,
					     CPLL);
		break;
	case PLL_NPLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[NPLL], priv->cru,
					     NPLL);
		break;
	case PLL_V0PLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[V0PLL], priv->cru,
					     V0PLL);
		break;
	case PLL_AUPLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[AUPLL], priv->cru,
					     AUPLL);
		break;
	case PLL_PPLL:
		rate = rockchip_pll_get_rate(&rk3588_pll_clks[PPLL], priv->cru,
					     PPLL);
		break;
	case ACLK_CENTER_ROOT:
	case PCLK_CENTER_ROOT:
	case HCLK_CENTER_ROOT:
	case ACLK_CENTER_LOW_ROOT:
		rate = rk3588_center_get_clk(priv, clk->id);
		break;
	case ACLK_TOP_ROOT:
	case PCLK_TOP_ROOT:
	case ACLK_LOW_TOP_ROOT:
		rate = rk3588_top_get_clk(priv, clk->id);
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C5:
	case CLK_I2C6:
	case CLK_I2C7:
	case CLK_I2C8:
		rate = rk3588_i2c_get_clk(priv, clk->id);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
	case CLK_SPI2:
	case CLK_SPI3:
	case CLK_SPI4:
		rate = rk3588_spi_get_clk(priv, clk->id);
		break;
	case CLK_PWM1:
	case CLK_PWM2:
	case CLK_PWM3:
	case CLK_PMU1PWM:
		rate = rk3588_pwm_get_clk(priv, clk->id);
		break;
	case CLK_SARADC:
	case CLK_TSADC:
		rate = rk3588_adc_get_clk(priv, clk->id);
		break;
	case CCLK_SRC_SDIO:
	case CCLK_EMMC:
	case BCLK_EMMC:
	case SCLK_SFC:
	case DCLK_DECOM:
		rate = rk3588_mmc_get_clk(priv, clk->id);
		break;
	case TMCLK_EMMC:
	case TCLK_WDT0:
		rate = OSC_HZ;
		break;
	case PCLK_PMU0_ROOT:
		rate = 100000000;
		break;
	case HCLK_PMU_CM0_ROOT:
		rate = 200000000;
		break;
	case ACLK_BUS_ROOT:
		rate = 375000000;
		break;
	case CLK_150M_SRC:
		rate = 150000000;
		break;
	case CLK_GPU:
		rate = 200000000;
		break;
#ifndef CONFIG_SPL_BUILD
	case CLK_AUX16M_0:
	case CLK_AUX16M_1:
		rate = rk3588_aux16m_get_clk(priv, clk->id);
		break;
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
	case ACLK_VOP_LOW_ROOT:
	case HCLK_VOP_ROOT:
		rate = rk3588_aclk_vop_get_clk(priv, clk->id);
		break;
	case DCLK_VOP0:
	case DCLK_VOP0_SRC:
	case DCLK_VOP1:
	case DCLK_VOP1_SRC:
	case DCLK_VOP2:
	case DCLK_VOP2_SRC:
	case DCLK_VOP3:
		rate = rk3588_dclk_vop_get_clk(priv, clk->id);
		break;
	case CLK_GMAC0_PTP_REF:
	case CLK_GMAC1_PTP_REF:
	case CLK_GMAC_125M:
	case CLK_GMAC_50M:
		rate = rk3588_gmac_get_clk(priv, clk->id);
		break;
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
	case SCLK_UART4:
	case SCLK_UART5:
	case SCLK_UART6:
	case SCLK_UART7:
	case SCLK_UART8:
	case SCLK_UART9:
		rate = rk3588_uart_get_rate(priv, clk->id);
		break;
	case CLK_REF_PIPE_PHY0:
	case CLK_REF_PIPE_PHY1:
	case CLK_REF_PIPE_PHY2:
		rate = rk3588_pciephy_get_rate(priv, clk->id);
		break;
#endif
	default:
		return -ENOENT;
	}

	return rate;
};

static ulong rk3588_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3588_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	if (!priv->ppll_hz) {
		priv->ppll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[PPLL],
						      priv->cru, PPLL);
	}

	switch (clk->id) {
	case PLL_CPLL:
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[CPLL], priv->cru,
					    CPLL, rate);
		priv->cpll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[CPLL],
						      priv->cru, CPLL);
		break;
	case PLL_GPLL:
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[GPLL], priv->cru,
					    GPLL, rate);
		priv->gpll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[GPLL],
						      priv->cru, GPLL);
		break;
	case PLL_NPLL:
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[NPLL], priv->cru,
					    NPLL, rate);
		break;
	case PLL_V0PLL:
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[V0PLL], priv->cru,
					    V0PLL, rate);
		priv->v0pll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[V0PLL],
						       priv->cru, V0PLL);
		break;
	case PLL_AUPLL:
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[AUPLL], priv->cru,
					    AUPLL, rate);
		priv->aupll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[AUPLL],
						       priv->cru, AUPLL);
		break;
	case PLL_PPLL:
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[PPLL], priv->cru,
					    PPLL, rate);
		priv->ppll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[PPLL],
						      priv->cru, PPLL);
		break;
	case ACLK_CENTER_ROOT:
	case PCLK_CENTER_ROOT:
	case HCLK_CENTER_ROOT:
	case ACLK_CENTER_LOW_ROOT:
		ret = rk3588_center_set_clk(priv, clk->id, rate);
		break;
	case ACLK_TOP_ROOT:
	case PCLK_TOP_ROOT:
	case ACLK_LOW_TOP_ROOT:
		ret = rk3588_top_set_clk(priv, clk->id, rate);
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C5:
	case CLK_I2C6:
	case CLK_I2C7:
	case CLK_I2C8:
		ret = rk3588_i2c_set_clk(priv, clk->id, rate);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
	case CLK_SPI2:
	case CLK_SPI3:
	case CLK_SPI4:
		ret = rk3588_spi_set_clk(priv, clk->id, rate);
		break;
	case CLK_PWM1:
	case CLK_PWM2:
	case CLK_PWM3:
	case CLK_PMU1PWM:
		ret = rk3588_pwm_set_clk(priv, clk->id, rate);
		break;
	case CLK_SARADC:
	case CLK_TSADC:
		ret = rk3588_adc_set_clk(priv, clk->id, rate);
		break;
	case CCLK_SRC_SDIO:
	case CCLK_EMMC:
	case BCLK_EMMC:
	case SCLK_SFC:
	case DCLK_DECOM:
		ret = rk3588_mmc_set_clk(priv, clk->id, rate);
		break;
	case TMCLK_EMMC:
	case TCLK_WDT0:
		ret = OSC_HZ;
		break;
	case PCLK_PMU0_ROOT:
	case CLK_GPU:
	case HCLK_PMU_CM0_ROOT:
	case ACLK_BUS_ROOT:
	case CLK_150M_SRC:
		ret = 0;
		break;
#ifndef CONFIG_SPL_BUILD
	case CLK_AUX16M_0:
	case CLK_AUX16M_1:
		ret = rk3588_aux16m_set_clk(priv, clk->id, rate);
		break;
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
	case ACLK_VOP_LOW_ROOT:
	case HCLK_VOP_ROOT:
		ret = rk3588_aclk_vop_set_clk(priv, clk->id, rate);
		break;
	case DCLK_VOP0:
	case DCLK_VOP0_SRC:
	case DCLK_VOP1:
	case DCLK_VOP1_SRC:
	case DCLK_VOP2:
	case DCLK_VOP2_SRC:
	case DCLK_VOP3:
		ret = rk3588_dclk_vop_set_clk(priv, clk->id, rate);
		break;
	case CLK_GMAC0_PTP_REF:
	case CLK_GMAC1_PTP_REF:
	case CLK_GMAC_125M:
	case CLK_GMAC_50M:
		ret = rk3588_gmac_set_clk(priv, clk->id, rate);
		break;
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
	case SCLK_UART4:
	case SCLK_UART5:
	case SCLK_UART6:
	case SCLK_UART7:
	case SCLK_UART8:
	case SCLK_UART9:
		ret = rk3588_uart_set_rate(priv, clk->id, rate);
		break;
	case CLK_REF_PIPE_PHY0:
	case CLK_REF_PIPE_PHY1:
	case CLK_REF_PIPE_PHY2:
		ret = rk3588_pciephy_set_rate(priv, clk->id, rate);
		break;
#endif
	default:
		return -ENOENT;
	}

	return ret;
};

#define ROCKCHIP_MMC_DELAY_SEL		BIT(10)
#define ROCKCHIP_MMC_DEGREE_MASK	0x3
#define ROCKCHIP_MMC_DELAYNUM_OFFSET	2
#define ROCKCHIP_MMC_DELAYNUM_MASK	(0xff << ROCKCHIP_MMC_DELAYNUM_OFFSET)

#define PSECS_PER_SEC 1000000000000LL
/*
 * Each fine delay is between 44ps-77ps. Assume each fine delay is 60ps to
 * simplify calculations. So 45degs could be anywhere between 33deg and 57.8deg.
 */
#define ROCKCHIP_MMC_DELAY_ELEMENT_PSEC 60

#if (IS_ENABLED(OF_CONTROL)) || (!IS_ENABLED(OF_PLATDATA))
static int __maybe_unused rk3588_dclk_vop_set_parent(struct clk *clk,
						     struct clk *parent)
{
	struct rk3588_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3588_cru *cru = priv->cru;
	u32 sel;
	const char *clock_dev_name = parent->dev->name;

	if (parent->id == PLL_V0PLL)
		sel = 2;
	else if (parent->id == PLL_GPLL)
		sel = 0;
	else if (parent->id == PLL_CPLL)
		sel = 1;
	else
		sel = 3;

	switch (clk->id) {
	case DCLK_VOP0_SRC:
		rk_clrsetreg(&cru->clksel_con[111], DCLK0_VOP_SRC_SEL_MASK,
			     sel << DCLK0_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VOP1_SRC:
		rk_clrsetreg(&cru->clksel_con[111], DCLK1_VOP_SRC_SEL_MASK,
			     sel << DCLK1_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VOP2_SRC:
		rk_clrsetreg(&cru->clksel_con[112], DCLK2_VOP_SRC_SEL_MASK,
			     sel << DCLK2_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VOP3:
		rk_clrsetreg(&cru->clksel_con[113], DCLK3_VOP_SRC_SEL_MASK,
			     sel << DCLK3_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VOP0:
		if (!strcmp(clock_dev_name, "hdmiphypll_clk0"))
			sel = 1;
		else if (!strcmp(clock_dev_name, "hdmiphypll_clk1"))
			sel = 2;
		else
			sel = 0;
		rk_clrsetreg(&cru->clksel_con[112], DCLK0_VOP_SEL_MASK,
			     sel << DCLK0_VOP_SEL_SHIFT);
		break;
	case DCLK_VOP1:
		if (!strcmp(clock_dev_name, "hdmiphypll_clk0"))
			sel = 1;
		else if (!strcmp(clock_dev_name, "hdmiphypll_clk1"))
			sel = 2;
		else
			sel = 0;
		rk_clrsetreg(&cru->clksel_con[112], DCLK1_VOP_SEL_MASK,
			     sel << DCLK1_VOP_SEL_SHIFT);
		break;
	case DCLK_VOP2:
		if (!strcmp(clock_dev_name, "hdmiphypll_clk0"))
			sel = 1;
		else if (!strcmp(clock_dev_name, "hdmiphypll_clk1"))
			sel = 2;
		else
			sel = 0;
		rk_clrsetreg(&cru->clksel_con[112], DCLK2_VOP_SEL_MASK,
			     sel << DCLK2_VOP_SEL_SHIFT);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int rk3588_clk_set_parent(struct clk *clk, struct clk *parent)
{
	switch (clk->id) {
	case DCLK_VOP0_SRC:
	case DCLK_VOP1_SRC:
	case DCLK_VOP2_SRC:
	case DCLK_VOP0:
	case DCLK_VOP1:
	case DCLK_VOP2:
	case DCLK_VOP3:
		return rk3588_dclk_vop_set_parent(clk, parent);
	default:
		return -ENOENT;
	}

	return 0;
}
#endif

static struct clk_ops rk3588_clk_ops = {
	.get_rate = rk3588_clk_get_rate,
	.set_rate = rk3588_clk_set_rate,
#if (IS_ENABLED(OF_CONTROL)) || (!IS_ENABLED(OF_PLATDATA))
	.set_parent = rk3588_clk_set_parent,
#endif
};

static void rk3588_clk_init(struct rk3588_clk_priv *priv)
{
	int ret, div;

	div = DIV_ROUND_UP(GPLL_HZ, 300 * MHz);
	rk_clrsetreg(&priv->cru->clksel_con[38],
		     ACLK_BUS_ROOT_SEL_MASK |
		     ACLK_BUS_ROOT_DIV_MASK,
		     div << ACLK_BUS_ROOT_DIV_SHIFT);

	if (priv->cpll_hz != CPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[CPLL], priv->cru,
					    CPLL, CPLL_HZ);
		if (!ret)
			priv->cpll_hz = CPLL_HZ;
	}
	if (priv->gpll_hz != GPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[GPLL], priv->cru,
					    GPLL, GPLL_HZ);
		if (!ret)
			priv->gpll_hz = GPLL_HZ;
	}

#ifdef CONFIG_PCI
	if (priv->ppll_hz != PPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[PPLL], priv->cru,
					    PPLL, PPLL_HZ);
		priv->ppll_hz = rockchip_pll_get_rate(&rk3588_pll_clks[PPLL],
						      priv->cru, PPLL);
	}
#endif
	rk_clrsetreg(&priv->cru->clksel_con[9],
		     ACLK_TOP_S400_SEL_MASK |
		     ACLK_TOP_S200_SEL_MASK,
		     (ACLK_TOP_S400_SEL_400M << ACLK_TOP_S400_SEL_SHIFT) |
		     (ACLK_TOP_S200_SEL_200M << ACLK_TOP_S200_SEL_SHIFT));
}

static int rk3588_clk_probe(struct udevice *dev)
{
	struct rk3588_clk_priv *priv = dev_get_priv(dev);
	int ret;

	priv->sync_kernel = false;

#ifdef CONFIG_SPL_BUILD
	rockchip_pll_set_rate(&rk3588_pll_clks[B0PLL], priv->cru,
			      B0PLL, LPLL_HZ);
	rockchip_pll_set_rate(&rk3588_pll_clks[B1PLL], priv->cru,
			      B1PLL, LPLL_HZ);
	if (!priv->armclk_enter_hz) {
		ret = rockchip_pll_set_rate(&rk3588_pll_clks[LPLL], priv->cru,
					    LPLL, LPLL_HZ);
		priv->armclk_enter_hz =
			rockchip_pll_get_rate(&rk3588_pll_clks[LPLL],
					      priv->cru, LPLL);
		priv->armclk_init_hz = priv->armclk_enter_hz;
	}
#endif

	priv->grf = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->grf))
		return PTR_ERR(priv->grf);

	rk3588_clk_init(priv);

	/* Process 'assigned-{clocks/clock-parents/clock-rates}' properties */
	ret = clk_set_defaults(dev, 1);
	if (ret)
		debug("%s clk_set_defaults failed %d\n", __func__, ret);
	else
		priv->sync_kernel = true;

	return 0;
}

static int rk3588_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk3588_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rk3588_clk_bind(struct udevice *dev)
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
		priv->glb_srst_fst_value = offsetof(struct rk3588_cru,
						    glb_srst_fst);
		priv->glb_srst_snd_value = offsetof(struct rk3588_cru,
						    glb_srsr_snd);
		dev_set_priv(sys_child, priv);
	}

#if CONFIG_IS_ENABLED(RESET_ROCKCHIP)
	ret = offsetof(struct rk3588_cru, softrst_con[0]);
	ret = rk3588_reset_bind_lut(dev, ret, 49158);
	if (ret)
		debug("Warning: software reset driver bind failed\n");
#endif

	return 0;
}

static const struct udevice_id rk3588_clk_ids[] = {
	{ .compatible = "rockchip,rk3588-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3588_cru) = {
	.name		= "rockchip_rk3588_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3588_clk_ids,
	.priv_auto	= sizeof(struct rk3588_clk_priv),
	.of_to_plat	= rk3588_clk_ofdata_to_platdata,
	.ops		= &rk3588_clk_ops,
	.bind		= rk3588_clk_bind,
	.probe		= rk3588_clk_probe,
};

#ifdef CONFIG_SPL_BUILD
#define SCRU_BASE			0xfd7d0000

static ulong rk3588_scru_clk_get_rate(struct clk *clk)
{
	u32 con, div, sel, parent;

	switch (clk->id) {
	case SCMI_CCLK_SD:
		con = readl(SCRU_BASE + RK3588_CLKSEL_CON(3));
		sel = (con & SCMI_CCLK_SD_SEL_MASK) >> SCMI_CCLK_SD_SEL_SHIFT;
		div = (con & SCMI_CCLK_SD_DIV_MASK) >> SCMI_CCLK_SD_DIV_SHIFT;
		if (sel == SCMI_CCLK_SD_SEL_GPLL)
			parent = GPLL_HZ;
		else if (sel == SCMI_CCLK_SD_SEL_SPLL)
			parent = SPLL_HZ;
		else
			parent = OSC_HZ;
		return DIV_TO_RATE(parent, div);
	case SCMI_HCLK_SD:
		con = readl(SCRU_BASE + RK3588_CLKSEL_CON(1));
		sel = (con & SCMI_HCLK_SD_SEL_MASK) >> SCMI_HCLK_SD_SEL_SHIFT;
		if (sel == SCMI_HCLK_SD_SEL_150M)
			return 150 * MHz;
		else if (sel == SCMI_HCLK_SD_SEL_100M)
			return 100 * MHz;
		else if (sel == SCMI_HCLK_SD_SEL_50M)
			return 50 * MHz;
		else
			return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rk3588_scru_clk_set_rate(struct clk *clk, ulong rate)
{
	u32 div, sel;

	switch (clk->id) {
	case SCMI_CCLK_SD:
		if ((OSC_HZ % rate) == 0) {
			sel = SCMI_CCLK_SD_SEL_24M;
			div = DIV_ROUND_UP(OSC_HZ, rate);
		} else if ((SPLL_HZ % rate) == 0) {
			sel = SCMI_CCLK_SD_SEL_SPLL;
			div = DIV_ROUND_UP(SPLL_HZ, rate);
		} else {
			sel = SCMI_CCLK_SD_SEL_GPLL;
			div = DIV_ROUND_UP(GPLL_HZ, rate);
		}
		rk_clrsetreg(SCRU_BASE + RK3588_CLKSEL_CON(3),
			     SCMI_CCLK_SD_SEL_MASK | SCMI_CCLK_SD_DIV_MASK,
			     sel << SCMI_CCLK_SD_SEL_SHIFT |
			     (div - 1) << SCMI_CCLK_SD_DIV_SHIFT);
		break;
	case SCMI_HCLK_SD:
		if (rate >= 150 * MHz)
			sel = SCMI_HCLK_SD_SEL_150M;
		else if (rate >= 100 * MHz)
			sel = SCMI_HCLK_SD_SEL_100M;
		else if (rate >= 50 * MHz)
			sel = SCMI_HCLK_SD_SEL_50M;
		else
			sel = SCMI_HCLK_SD_SEL_24M;
		rk_clrsetreg(SCRU_BASE + RK3588_CLKSEL_CON(1),
			     SCMI_HCLK_SD_SEL_MASK,
			     sel << SCMI_HCLK_SD_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3588_scru_clk_get_rate(clk);
}

static const struct clk_ops rk3588_scru_clk_ops = {
	.get_rate = rk3588_scru_clk_get_rate,
	.set_rate = rk3588_scru_clk_set_rate,
};

U_BOOT_DRIVER(rockchip_rk3588_scru) = {
	.name = "rockchip_rk3588_scru",
	.id = UCLASS_CLK,
	.ops = &rk3588_scru_clk_ops,
};

static int rk3588_scmi_spl_glue_bind(struct udevice *dev)
{
	ofnode node;
	u32 protocol_id;
	const char *name;

	dev_for_each_subnode(node, dev) {
		if (!ofnode_is_enabled(node))
			continue;

		if (ofnode_read_u32(node, "reg", &protocol_id))
			continue;

		if (protocol_id != SCMI_PROTOCOL_ID_CLOCK)
			continue;

		name = ofnode_get_name(node);
		return device_bind_driver_to_node(dev, "rockchip_rk3588_scru",
						  name, node, NULL);
	}

	return -ENOENT;
}

static const struct udevice_id rk3588_scmi_spl_glue_ids[] = {
	{ .compatible = "arm,scmi-smc" },
	{ }
};

U_BOOT_DRIVER(rk3588_scmi_spl_glue) = {
	.name		= "rk3588_scmi_spl_glue",
	.id		= UCLASS_NOP,
	.of_match	= rk3588_scmi_spl_glue_ids,
	.bind		= rk3588_scmi_spl_glue_bind,
};
#endif
