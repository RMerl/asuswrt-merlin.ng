// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Based on Rockchip's drivers/power/pmic/pmic_rk808.c:
 * Copyright (C) 2012 rockchips
 * zyw <zyw@rock-chips.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <linux/delay.h>
#include <power/rk8xx_pmic.h>
#include <power/pmic.h>
#include <power/regulator.h>

#ifndef CONFIG_SPL_BUILD
#define ENABLE_DRIVER
#endif

/* Not used or exisit register and configure */
#define NA			0xff

/* Field Definitions */
#define RK808_BUCK_VSEL_MASK	0x3f
#define RK808_BUCK4_VSEL_MASK	0xf
#define RK808_LDO_VSEL_MASK	0x1f

/* RK809 BUCK5 */
#define RK809_BUCK5_CONFIG(n)		(0xde + (n) * 1)
#define RK809_BUCK5_VSEL_MASK		0x07

/* RK817 BUCK */
#define RK817_BUCK_ON_VSEL(n)		(0xbb + 3 * ((n) - 1))
#define RK817_BUCK_SLP_VSEL(n)		(0xbc + 3 * ((n) - 1))
#define RK817_BUCK_VSEL_MASK		0x7f
#define RK817_BUCK_CONFIG(i)		(0xba + (i) * 3)

/* RK817 LDO */
#define RK817_LDO_ON_VSEL(n)		(0xcc + 2 * ((n) - 1))
#define RK817_LDO_SLP_VSEL(n)		(0xcd + 2 * ((n) - 1))
#define RK817_LDO_VSEL_MASK		0x7f

/* RK817 ENABLE */
#define RK817_POWER_EN(n)		(0xb1 + (n))
#define RK817_POWER_SLP_EN(n)		(0xb5 + (n))

#define RK818_BUCK_VSEL_MASK		0x3f
#define RK818_BUCK4_VSEL_MASK		0x1f
#define RK818_LDO_VSEL_MASK		0x1f
#define RK818_LDO3_ON_VSEL_MASK	0xf
#define RK818_BOOST_ON_VSEL_MASK	0xe0
#define RK818_USB_ILIM_SEL_MASK		0x0f
#define RK818_USB_CHG_SD_VSEL_MASK	0x70

/*
 * Ramp delay
 */
#define RK805_RAMP_RATE_OFFSET		3
#define RK805_RAMP_RATE_MASK		(3 << RK805_RAMP_RATE_OFFSET)
#define RK805_RAMP_RATE_3MV_PER_US	(0 << RK805_RAMP_RATE_OFFSET)
#define RK805_RAMP_RATE_6MV_PER_US	(1 << RK805_RAMP_RATE_OFFSET)
#define RK805_RAMP_RATE_12_5MV_PER_US	(2 << RK805_RAMP_RATE_OFFSET)
#define RK805_RAMP_RATE_25MV_PER_US	(3 << RK805_RAMP_RATE_OFFSET)

#define RK808_RAMP_RATE_OFFSET		3
#define RK808_RAMP_RATE_MASK		(3 << RK808_RAMP_RATE_OFFSET)
#define RK808_RAMP_RATE_2MV_PER_US	(0 << RK808_RAMP_RATE_OFFSET)
#define RK808_RAMP_RATE_4MV_PER_US	(1 << RK808_RAMP_RATE_OFFSET)
#define RK808_RAMP_RATE_6MV_PER_US	(2 << RK808_RAMP_RATE_OFFSET)
#define RK808_RAMP_RATE_10MV_PER_US	(3 << RK808_RAMP_RATE_OFFSET)

#define RK817_RAMP_RATE_OFFSET		6
#define RK817_RAMP_RATE_MASK		(0x3 << RK817_RAMP_RATE_OFFSET)
#define RK817_RAMP_RATE_3MV_PER_US	(0x0 << RK817_RAMP_RATE_OFFSET)
#define RK817_RAMP_RATE_6_3MV_PER_US	(0x1 << RK817_RAMP_RATE_OFFSET)
#define RK817_RAMP_RATE_12_5MV_PER_US	(0x2 << RK817_RAMP_RATE_OFFSET)
#define RK817_RAMP_RATE_25MV_PER_US	(0x3 << RK817_RAMP_RATE_OFFSET)

struct rk8xx_reg_info {
	uint min_uv;
	uint step_uv;
	u8 vsel_reg;
	u8 vsel_sleep_reg;
	u8 config_reg;
	u8 vsel_mask;
	u8 min_sel;
	u8 max_sel;
};

static const struct rk8xx_reg_info rk808_buck[] = {
	{  712500,  12500, REG_BUCK1_ON_VSEL, REG_BUCK1_SLP_VSEL, REG_BUCK1_CONFIG,  RK808_BUCK_VSEL_MASK, 0x00, 0x3f },
	{  712500,  12500, REG_BUCK2_ON_VSEL, REG_BUCK2_SLP_VSEL, REG_BUCK2_CONFIG,  RK808_BUCK_VSEL_MASK, 0x00, 0x3f },
	{      NA,     NA,		  NA,		      NA, REG_BUCK3_CONFIG,		       NA,   NA,   NA },
	{ 1800000, 100000, REG_BUCK4_ON_VSEL, REG_BUCK4_SLP_VSEL, REG_BUCK4_CONFIG, RK808_BUCK4_VSEL_MASK, 0x00, 0x0f },
};

static const struct rk8xx_reg_info rk816_buck[] = {
	/* buck 1 */
	{  712500,  12500, REG_BUCK1_ON_VSEL, REG_BUCK1_SLP_VSEL, REG_BUCK1_CONFIG,  RK818_BUCK_VSEL_MASK, 0x00, 0x3b },
	{ 1800000, 200000, REG_BUCK1_ON_VSEL, REG_BUCK1_SLP_VSEL, REG_BUCK1_CONFIG,  RK818_BUCK_VSEL_MASK, 0x3c, 0x3e },
	{ 2300000,	0, REG_BUCK1_ON_VSEL, REG_BUCK1_SLP_VSEL, REG_BUCK1_CONFIG,  RK818_BUCK_VSEL_MASK, 0x3f, 0x3f },
	/* buck 2 */
	{  712500,  12500, REG_BUCK2_ON_VSEL, REG_BUCK2_SLP_VSEL, REG_BUCK2_CONFIG,  RK818_BUCK_VSEL_MASK, 0x00, 0x3b },
	{ 1800000, 200000, REG_BUCK2_ON_VSEL, REG_BUCK2_SLP_VSEL, REG_BUCK2_CONFIG,  RK818_BUCK_VSEL_MASK, 0x3c, 0x3e },
	{ 2300000,	0, REG_BUCK2_ON_VSEL, REG_BUCK2_SLP_VSEL, REG_BUCK2_CONFIG,  RK818_BUCK_VSEL_MASK, 0x3f, 0x3f },
	/* buck 3 */
	{      NA,     NA,		  NA,		      NA, REG_BUCK3_CONFIG,                    NA,   NA,   NA },
	/* buck 4 */
	{  800000, 100000, REG_BUCK4_ON_VSEL, REG_BUCK4_SLP_VSEL, REG_BUCK4_CONFIG, RK818_BUCK4_VSEL_MASK, 0x00, 0x1f },
};

static const struct rk8xx_reg_info rk809_buck5[] = {
	/* buck 5 */
	{ 1500000,	0, RK809_BUCK5_CONFIG(0), RK809_BUCK5_CONFIG(1), NA, RK809_BUCK5_VSEL_MASK, 0x00, 0x00 },
	{ 1800000, 200000, RK809_BUCK5_CONFIG(0), RK809_BUCK5_CONFIG(1), NA, RK809_BUCK5_VSEL_MASK, 0x01, 0x03 },
	{ 2800000, 200000, RK809_BUCK5_CONFIG(0), RK809_BUCK5_CONFIG(1), NA, RK809_BUCK5_VSEL_MASK, 0x04, 0x05 },
	{ 3300000, 300000, RK809_BUCK5_CONFIG(0), RK809_BUCK5_CONFIG(1), NA, RK809_BUCK5_VSEL_MASK, 0x06, 0x07 },
};

static const struct rk8xx_reg_info rk817_buck[] = {
	/* buck 1 */
	{  500000,  12500, RK817_BUCK_ON_VSEL(1), RK817_BUCK_SLP_VSEL(1), RK817_BUCK_CONFIG(1), RK817_BUCK_VSEL_MASK, 0x00, 0x4f },
	{ 1500000, 100000, RK817_BUCK_ON_VSEL(1), RK817_BUCK_SLP_VSEL(1), RK817_BUCK_CONFIG(1), RK817_BUCK_VSEL_MASK, 0x50, 0x58 },
	{ 2400000,	0, RK817_BUCK_ON_VSEL(1), RK817_BUCK_SLP_VSEL(1), RK817_BUCK_CONFIG(1), RK817_BUCK_VSEL_MASK, 0x59, 0x7f },
	/* buck 2 */
	{  500000,  12500, RK817_BUCK_ON_VSEL(2), RK817_BUCK_SLP_VSEL(2), RK817_BUCK_CONFIG(2), RK817_BUCK_VSEL_MASK, 0x00, 0x4f },
	{ 1500000, 100000, RK817_BUCK_ON_VSEL(2), RK817_BUCK_SLP_VSEL(2), RK817_BUCK_CONFIG(2), RK817_BUCK_VSEL_MASK, 0x50, 0x58 },
	{ 2400000,	0, RK817_BUCK_ON_VSEL(2), RK817_BUCK_SLP_VSEL(2), RK817_BUCK_CONFIG(2), RK817_BUCK_VSEL_MASK, 0x59, 0x7f },
	/* buck 3 */
	{  500000,  12500, RK817_BUCK_ON_VSEL(3), RK817_BUCK_SLP_VSEL(3), RK817_BUCK_CONFIG(3), RK817_BUCK_VSEL_MASK, 0x00, 0x4f },
	{ 1500000, 100000, RK817_BUCK_ON_VSEL(3), RK817_BUCK_SLP_VSEL(3), RK817_BUCK_CONFIG(3), RK817_BUCK_VSEL_MASK, 0x50, 0x58 },
	{ 2400000,	0, RK817_BUCK_ON_VSEL(3), RK817_BUCK_SLP_VSEL(3), RK817_BUCK_CONFIG(3), RK817_BUCK_VSEL_MASK, 0x59, 0x7f },
	/* buck 4 */
	{  500000,  12500, RK817_BUCK_ON_VSEL(4), RK817_BUCK_SLP_VSEL(4), RK817_BUCK_CONFIG(4), RK817_BUCK_VSEL_MASK, 0x00, 0x4f },
	{ 1500000, 100000, RK817_BUCK_ON_VSEL(4), RK817_BUCK_SLP_VSEL(4), RK817_BUCK_CONFIG(4), RK817_BUCK_VSEL_MASK, 0x50, 0x62 },
	{ 3400000,	0, RK817_BUCK_ON_VSEL(4), RK817_BUCK_SLP_VSEL(4), RK817_BUCK_CONFIG(4), RK817_BUCK_VSEL_MASK, 0x63, 0x7f },
};

static const struct rk8xx_reg_info rk818_buck[] = {
	{ 712500,   12500, REG_BUCK1_ON_VSEL, REG_BUCK1_SLP_VSEL, REG_BUCK1_CONFIG,  RK818_BUCK_VSEL_MASK, 0x00, 0x3f },
	{ 712500,   12500, REG_BUCK2_ON_VSEL, REG_BUCK2_SLP_VSEL, REG_BUCK2_CONFIG,  RK818_BUCK_VSEL_MASK, 0x00, 0x3f },
	{     NA,      NA,		  NA,		      NA, REG_BUCK3_CONFIG,		       NA,   NA,   NA },
	{ 1800000, 100000, REG_BUCK4_ON_VSEL, REG_BUCK4_SLP_VSEL, REG_BUCK4_CONFIG, RK818_BUCK4_VSEL_MASK, 0x00, 0x1f },
};

#ifdef ENABLE_DRIVER
static const struct rk8xx_reg_info rk808_ldo[] = {
	{ 1800000, 100000, REG_LDO1_ON_VSEL, REG_LDO1_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO2_ON_VSEL, REG_LDO2_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
	{  800000, 100000, REG_LDO3_ON_VSEL, REG_LDO3_SLP_VSEL, NA, RK808_BUCK4_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO4_ON_VSEL, REG_LDO4_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO5_ON_VSEL, REG_LDO5_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
	{  800000, 100000, REG_LDO6_ON_VSEL, REG_LDO6_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
	{  800000, 100000, REG_LDO7_ON_VSEL, REG_LDO7_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO8_ON_VSEL, REG_LDO8_SLP_VSEL, NA, RK808_LDO_VSEL_MASK, },
};

static const struct rk8xx_reg_info rk816_ldo[] = {
	{ 800000, 100000, REG_LDO1_ON_VSEL, REG_LDO1_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 800000, 100000, REG_LDO2_ON_VSEL, REG_LDO2_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 800000, 100000, REG_LDO3_ON_VSEL, REG_LDO3_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 800000, 100000, REG_LDO4_ON_VSEL, REG_LDO4_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 800000, 100000, REG_LDO5_ON_VSEL, REG_LDO5_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 800000, 100000, REG_LDO6_ON_VSEL, REG_LDO6_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
};

static const struct rk8xx_reg_info rk817_ldo[] = {
	/* ldo1 */
	{  600000, 25000, RK817_LDO_ON_VSEL(1), RK817_LDO_SLP_VSEL(1), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(1), RK817_LDO_SLP_VSEL(1), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo2 */
	{  600000, 25000, RK817_LDO_ON_VSEL(2), RK817_LDO_SLP_VSEL(2), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(2), RK817_LDO_SLP_VSEL(2), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo3 */
	{  600000, 25000, RK817_LDO_ON_VSEL(3), RK817_LDO_SLP_VSEL(3), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(3), RK817_LDO_SLP_VSEL(3), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo4 */
	{  600000, 25000, RK817_LDO_ON_VSEL(4), RK817_LDO_SLP_VSEL(4), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(4), RK817_LDO_SLP_VSEL(4), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo5 */
	{  600000, 25000, RK817_LDO_ON_VSEL(5), RK817_LDO_SLP_VSEL(5), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(5), RK817_LDO_SLP_VSEL(5), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo6 */
	{  600000, 25000, RK817_LDO_ON_VSEL(6), RK817_LDO_SLP_VSEL(6), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(6), RK817_LDO_SLP_VSEL(6), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo7 */
	{  600000, 25000, RK817_LDO_ON_VSEL(7), RK817_LDO_SLP_VSEL(7), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(7), RK817_LDO_SLP_VSEL(7), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo8 */
	{  600000, 25000, RK817_LDO_ON_VSEL(8), RK817_LDO_SLP_VSEL(8), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(8), RK817_LDO_SLP_VSEL(8), NA, RK817_LDO_VSEL_MASK, 0x70, },
	/* ldo9 */
	{  600000, 25000, RK817_LDO_ON_VSEL(9), RK817_LDO_SLP_VSEL(9), NA, RK817_LDO_VSEL_MASK, 0x00, },
	{ 3400000,     0, RK817_LDO_ON_VSEL(9), RK817_LDO_SLP_VSEL(9), NA, RK817_LDO_VSEL_MASK, 0x70, },
};

static const struct rk8xx_reg_info rk818_ldo[] = {
	{ 1800000, 100000, REG_LDO1_ON_VSEL, REG_LDO1_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO2_ON_VSEL, REG_LDO2_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{  800000, 100000, REG_LDO3_ON_VSEL, REG_LDO3_SLP_VSEL, NA, RK818_LDO3_ON_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO4_ON_VSEL, REG_LDO4_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO5_ON_VSEL, REG_LDO5_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{  800000, 100000, REG_LDO6_ON_VSEL, REG_LDO6_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{  800000, 100000, REG_LDO7_ON_VSEL, REG_LDO7_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
	{ 1800000, 100000, REG_LDO8_ON_VSEL, REG_LDO8_SLP_VSEL, NA, RK818_LDO_VSEL_MASK, },
};
#endif

static const u16 rk818_chrg_cur_input_array[] = {
	450, 800, 850, 1000, 1250, 1500, 1750, 2000, 2250, 2500, 2750, 3000
};

static const uint rk818_chrg_shutdown_vsel_array[] = {
	2780000, 2850000, 2920000, 2990000, 3060000, 3130000, 3190000, 3260000
};

static const struct rk8xx_reg_info *get_buck_reg(struct udevice *pmic,
						 int num, int uvolt)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		switch (num) {
		case 0:
		case 1:
			if (uvolt <= 1450000)
				return &rk816_buck[num * 3 + 0];
			else if (uvolt <= 2200000)
				return &rk816_buck[num * 3 + 1];
			else
				return &rk816_buck[num * 3 + 2];
		default:
			return &rk816_buck[num + 4];
		}

	case RK809_ID:
	case RK817_ID:
		switch (num) {
		case 0 ... 2:
			if (uvolt < 1500000)
				return &rk817_buck[num * 3 + 0];
			else if (uvolt < 2400000)
				return &rk817_buck[num * 3 + 1];
			else
				return &rk817_buck[num * 3 + 2];
		case 3:
			if (uvolt < 1500000)
				return &rk817_buck[num * 3 + 0];
			else if (uvolt < 3400000)
				return &rk817_buck[num * 3 + 1];
			else
				return &rk817_buck[num * 3 + 2];
		/* BUCK5 for RK809 */
		default:
			if (uvolt < 1800000)
				return &rk809_buck5[0];
			else if (uvolt < 2800000)
				return &rk809_buck5[1];
			else if (uvolt < 3300000)
				return &rk809_buck5[2];
			else
				return &rk809_buck5[3];
		}
	case RK818_ID:
		return &rk818_buck[num];
	default:
		return &rk808_buck[num];
	}
}

static int _buck_set_value(struct udevice *pmic, int buck, int uvolt)
{
	const struct rk8xx_reg_info *info = get_buck_reg(pmic, buck, uvolt);
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	int mask = info->vsel_mask;
	int val;

	if (info->vsel_reg == NA)
		return -ENOSYS;

	if (info->step_uv == 0)	/* Fixed voltage */
		val = info->min_sel;
	else
		val = ((uvolt - info->min_uv) / info->step_uv) + info->min_sel;

	debug("%s: volt=%d, buck=%d, reg=0x%x, mask=0x%x, val=0x%x\n",
	      __func__, uvolt, buck + 1, info->vsel_reg, mask, val);

	if (priv->variant == RK816_ID) {
		pmic_clrsetbits(pmic, info->vsel_reg, mask, val);
		return pmic_clrsetbits(pmic, RK816_REG_DCDC_EN2,
				       1 << 7, 1 << 7);
	} else {
		return pmic_clrsetbits(pmic, info->vsel_reg, mask, val);
	}
}

static int _buck_set_enable(struct udevice *pmic, int buck, bool enable)
{
	uint mask, value, en_reg;
	int ret = 0;
	struct rk8xx_priv *priv = dev_get_priv(pmic);

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		if (buck >= 4) {
			buck -= 4;
			en_reg = RK816_REG_DCDC_EN2;
		} else {
			en_reg = RK816_REG_DCDC_EN1;
		}
		if (enable)
			value = ((1 << buck) | (1 << (buck + 4)));
		else
			value = ((0 << buck) | (1 << (buck + 4)));
		ret = pmic_reg_write(pmic, en_reg, value);
		break;

	case RK808_ID:
	case RK818_ID:
		mask = 1 << buck;
		if (enable) {
			ret = pmic_clrsetbits(pmic, REG_DCDC_ILMAX,
					      0, 3 << (buck * 2));
			if (ret)
				return ret;
		}
		ret = pmic_clrsetbits(pmic, REG_DCDC_EN, mask,
				      enable ? mask : 0);
		break;
	case RK809_ID:
	case RK817_ID:
		if (buck < 4) {
			if (enable)
				value = ((1 << buck) | (1 << (buck + 4)));
			else
				value = ((0 << buck) | (1 << (buck + 4)));
			ret = pmic_reg_write(pmic, RK817_POWER_EN(0), value);
		/* BUCK5 for RK809 */
		} else {
			if (enable)
				value = ((1 << 1) | (1 << 5));
			else
				value = ((0 << 1) | (1 << 5));
			ret = pmic_reg_write(pmic, RK817_POWER_EN(3), value);
		}
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

#ifdef ENABLE_DRIVER
static int _buck_set_suspend_value(struct udevice *pmic, int buck, int uvolt)
{
	const struct rk8xx_reg_info *info = get_buck_reg(pmic, buck, uvolt);
	int mask = info->vsel_mask;
	int val;

	if (info->vsel_sleep_reg == NA)
		return -ENOSYS;

	if (info->step_uv == 0)
		val = info->min_sel;
	else
		val = ((uvolt - info->min_uv) / info->step_uv) + info->min_sel;

	debug("%s: volt=%d, buck=%d, reg=0x%x, mask=0x%x, val=0x%x\n",
	      __func__, uvolt, buck + 1, info->vsel_sleep_reg, mask, val);

	return pmic_clrsetbits(pmic, info->vsel_sleep_reg, mask, val);
}

static int _buck_get_enable(struct udevice *pmic, int buck)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	uint mask = 0;
	int ret = 0;

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		if (buck >= 4) {
			mask = 1 << (buck - 4);
			ret = pmic_reg_read(pmic, RK816_REG_DCDC_EN2);
		} else {
			mask = 1 << buck;
			ret = pmic_reg_read(pmic, RK816_REG_DCDC_EN1);
		}
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << buck;
		ret = pmic_reg_read(pmic, REG_DCDC_EN);
		if (ret < 0)
			return ret;
		break;
	case RK809_ID:
	case RK817_ID:
		if (buck < 4) {
			mask = 1 << buck;
			ret = pmic_reg_read(pmic, RK817_POWER_EN(0));
		/* BUCK5 for RK809 */
		} else {
			mask = 1 << 1;
			ret = pmic_reg_read(pmic, RK817_POWER_EN(3));
		}
		break;
	}

	if (ret < 0)
		return ret;

	return ret & mask ? true : false;
}

static int _buck_set_suspend_enable(struct udevice *pmic, int buck, bool enable)
{
	uint mask = 0;
	int ret;
	struct rk8xx_priv *priv = dev_get_priv(pmic);

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		mask = 1 << buck;
		ret = pmic_clrsetbits(pmic, RK816_REG_DCDC_SLP_EN, mask,
				      enable ? mask : 0);
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << buck;
		ret = pmic_clrsetbits(pmic, REG_SLEEP_SET_OFF1, mask,
				      enable ? 0 : mask);
		break;
	case RK809_ID:
	case RK817_ID:
		if (buck < 4)
			mask = 1 << buck;
		else
			mask = 1 << 5;	/* BUCK5 for RK809 */
		ret = pmic_clrsetbits(pmic, RK817_POWER_SLP_EN(0), mask,
				      enable ? mask : 0);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int _buck_get_suspend_enable(struct udevice *pmic, int buck)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	int ret, val;
	uint mask = 0;

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		mask = 1 << buck;
		val = pmic_reg_read(pmic, RK816_REG_DCDC_SLP_EN);
		if (val < 0)
			return val;
		ret = val & mask ? 1 : 0;
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << buck;
		val = pmic_reg_read(pmic, REG_SLEEP_SET_OFF1);
		if (val < 0)
			return val;
		ret = val & mask ? 0 : 1;
		break;
	case RK809_ID:
	case RK817_ID:
		if (buck < 4)
			mask = 1 << buck;
		else
			mask = 1 << 5;	/* BUCK5 for RK809 */

		val = pmic_reg_read(pmic, RK817_POWER_SLP_EN(0));
		if (val < 0)
			return val;
		ret = val & mask ? 1 : 0;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct rk8xx_reg_info *get_ldo_reg(struct udevice *pmic,
						int num, int uvolt)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		return &rk816_ldo[num];
	case RK809_ID:
	case RK817_ID:
		if (uvolt < 3400000)
			return &rk817_ldo[num * 2 + 0];
		else
			return &rk817_ldo[num * 2 + 1];
	case RK818_ID:
		return &rk818_ldo[num];
	default:
		return &rk808_ldo[num];
	}
}

static int _ldo_get_enable(struct udevice *pmic, int ldo)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	uint mask = 0;
	int ret = 0;

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		if (ldo >= 4) {
			mask = 1 << (ldo - 4);
			ret = pmic_reg_read(pmic, RK816_REG_LDO_EN2);
		} else {
			mask = 1 << ldo;
			ret = pmic_reg_read(pmic, RK816_REG_LDO_EN1);
		}
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << ldo;
		ret = pmic_reg_read(pmic, REG_LDO_EN);
		if (ret < 0)
			return ret;
		break;
	case RK809_ID:
	case RK817_ID:
		if (ldo < 4) {
			mask = 1 << ldo;
			ret = pmic_reg_read(pmic, RK817_POWER_EN(1));
		} else if (ldo < 8) {
			mask = 1 << (ldo - 4);
			ret = pmic_reg_read(pmic, RK817_POWER_EN(2));
		} else if (ldo == 8) {
			mask = 1 << 0;
			ret = pmic_reg_read(pmic, RK817_POWER_EN(3));
		} else {
			return false;
		}
		break;
	}

	if (ret < 0)
		return ret;

	return ret & mask ? true : false;
}

static int _ldo_set_enable(struct udevice *pmic, int ldo, bool enable)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	uint mask, value, en_reg;
	int ret = 0;

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		if (ldo >= 4) {
			ldo -= 4;
			en_reg = RK816_REG_LDO_EN2;
		} else {
			en_reg = RK816_REG_LDO_EN1;
		}
		if (enable)
			value = ((1 << ldo) | (1 << (ldo + 4)));
		else
			value = ((0 << ldo) | (1 << (ldo + 4)));

		ret = pmic_reg_write(pmic, en_reg, value);
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << ldo;
		ret = pmic_clrsetbits(pmic, REG_LDO_EN, mask,
				       enable ? mask : 0);
		break;
	case RK809_ID:
	case RK817_ID:
		if (ldo < 4) {
			en_reg = RK817_POWER_EN(1);
		} else if (ldo < 8) {
			ldo -= 4;
			en_reg = RK817_POWER_EN(2);
		} else if (ldo == 8) {
			ldo = 0;	/* BIT 0 */
			en_reg = RK817_POWER_EN(3);
		} else {
			return -EINVAL;
		}
		if (enable)
			value = ((1 << ldo) | (1 << (ldo + 4)));
		else
			value = ((0 << ldo) | (1 << (ldo + 4)));
		ret = pmic_reg_write(pmic, en_reg, value);
		break;
	}

	if (enable)
		udelay(500);

	return ret;
}

static int _ldo_set_suspend_enable(struct udevice *pmic, int ldo, bool enable)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	uint mask;
	int ret = 0;

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		mask = 1 << ldo;
		ret = pmic_clrsetbits(pmic, RK816_REG_LDO_SLP_EN, mask,
				      enable ? mask : 0);
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << ldo;
		ret = pmic_clrsetbits(pmic, REG_SLEEP_SET_OFF2, mask,
				      enable ? 0 : mask);
		break;
	case RK809_ID:
	case RK817_ID:
		if (ldo == 8) {
			mask = 1 << 4;	/* LDO9 */
			ret = pmic_clrsetbits(pmic, RK817_POWER_SLP_EN(0), mask,
					      enable ? mask : 0);
		} else {
			mask = 1 << ldo;
			ret = pmic_clrsetbits(pmic, RK817_POWER_SLP_EN(1), mask,
					      enable ? mask : 0);
		}
		break;
	}

	return ret;
}

static int _ldo_get_suspend_enable(struct udevice *pmic, int ldo)
{
	struct rk8xx_priv *priv = dev_get_priv(pmic);
	int val, ret = 0;
	uint mask;

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
		mask = 1 << ldo;
		val = pmic_reg_read(pmic, RK816_REG_LDO_SLP_EN);
		if (val < 0)
			return val;
		ret = val & mask ? 1 : 0;
		break;
	case RK808_ID:
	case RK818_ID:
		mask = 1 << ldo;
		val = pmic_reg_read(pmic, REG_SLEEP_SET_OFF2);
		if (val < 0)
			return val;
		ret = val & mask ? 0 : 1;
		break;
	case RK809_ID:
	case RK817_ID:
		if (ldo == 8) {
			mask = 1 << 4;	/* LDO9 */
			val = pmic_reg_read(pmic, RK817_POWER_SLP_EN(0));
			if (val < 0)
				return val;
			ret = val & mask ? 1 : 0;
		} else {
			mask = 1 << ldo;
			val = pmic_reg_read(pmic, RK817_POWER_SLP_EN(1));
			if (val < 0)
				return val;
			ret = val & mask ? 1 : 0;
		}
		break;
	}

	return ret;
}

static int buck_get_value(struct udevice *dev)
{
	int buck = dev->driver_data - 1;
	const struct rk8xx_reg_info *info = get_buck_reg(dev->parent, buck, 0);
	int mask = info->vsel_mask;
	int ret, val;

	if (info->vsel_reg == NA)
		return -ENOSYS;

	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;

	val = ret & mask;
	while (val > info->max_sel)
		info++;

	return info->min_uv + (val - info->min_sel) * info->step_uv;
}

static int buck_set_value(struct udevice *dev, int uvolt)
{
	int buck = dev->driver_data - 1;

	return _buck_set_value(dev->parent, buck, uvolt);
}

static int buck_get_suspend_value(struct udevice *dev)
{
	int buck = dev->driver_data - 1;
	const struct rk8xx_reg_info *info = get_buck_reg(dev->parent, buck, 0);
	int mask = info->vsel_mask;
	int ret, val;

	if (info->vsel_sleep_reg == NA)
		return -ENOSYS;

	ret = pmic_reg_read(dev->parent, info->vsel_sleep_reg);
	if (ret < 0)
		return ret;

	val = ret & mask;
	while (val > info->max_sel)
		info++;

	return info->min_uv + (val - info->min_sel) * info->step_uv;
}

static int buck_set_suspend_value(struct udevice *dev, int uvolt)
{
	int buck = dev->driver_data - 1;

	return _buck_set_suspend_value(dev->parent, buck, uvolt);
}

static int buck_set_enable(struct udevice *dev, bool enable)
{
	int buck = dev->driver_data - 1;

	return _buck_set_enable(dev->parent, buck, enable);
}

static int buck_set_suspend_enable(struct udevice *dev, bool enable)
{
	int buck = dev->driver_data - 1;

	return _buck_set_suspend_enable(dev->parent, buck, enable);
}

static int buck_get_suspend_enable(struct udevice *dev)
{
	int buck = dev->driver_data - 1;

	return _buck_get_suspend_enable(dev->parent, buck);
}

static int buck_get_enable(struct udevice *dev)
{
	int buck = dev->driver_data - 1;

	return _buck_get_enable(dev->parent, buck);
}

static int ldo_get_value(struct udevice *dev)
{
	int ldo = dev->driver_data - 1;
	const struct rk8xx_reg_info *info = get_ldo_reg(dev->parent, ldo, 0);
	int mask = info->vsel_mask;
	int ret, val;

	if (info->vsel_reg == NA)
		return -ENOSYS;
	ret = pmic_reg_read(dev->parent, info->vsel_reg);
	if (ret < 0)
		return ret;
	val = ret & mask;

	return info->min_uv + val * info->step_uv;
}

static int ldo_set_value(struct udevice *dev, int uvolt)
{
	int ldo = dev->driver_data - 1;
	const struct rk8xx_reg_info *info = get_ldo_reg(dev->parent, ldo, uvolt);
	int mask = info->vsel_mask;
	int val;

	if (info->vsel_reg == NA)
		return -ENOSYS;

	if (info->step_uv == 0)
		val = info->min_sel;
	else
		val = ((uvolt - info->min_uv) / info->step_uv) + info->min_sel;

	debug("%s: volt=%d, ldo=%d, reg=0x%x, mask=0x%x, val=0x%x\n",
	      __func__, uvolt, ldo + 1, info->vsel_reg, mask, val);

	return pmic_clrsetbits(dev->parent, info->vsel_reg, mask, val);
}

static int ldo_set_suspend_value(struct udevice *dev, int uvolt)
{
	int ldo = dev->driver_data - 1;
	const struct rk8xx_reg_info *info = get_ldo_reg(dev->parent, ldo, uvolt);
	int mask = info->vsel_mask;
	int val;

	if (info->vsel_sleep_reg == NA)
		return -ENOSYS;

	if (info->step_uv == 0)
		val = info->min_sel;
	else
		val = ((uvolt - info->min_uv) / info->step_uv) + info->min_sel;

	debug("%s: volt=%d, ldo=%d, reg=0x%x, mask=0x%x, val=0x%x\n",
	      __func__, uvolt, ldo + 1, info->vsel_sleep_reg, mask, val);

	return pmic_clrsetbits(dev->parent, info->vsel_sleep_reg, mask, val);
}

static int ldo_get_suspend_value(struct udevice *dev)
{
	int ldo = dev->driver_data - 1;
	const struct rk8xx_reg_info *info = get_ldo_reg(dev->parent, ldo, 0);
	int mask = info->vsel_mask;
	int val, ret;

	if (info->vsel_sleep_reg == NA)
		return -ENOSYS;

	ret = pmic_reg_read(dev->parent, info->vsel_sleep_reg);
	if (ret < 0)
		return ret;

	val = ret & mask;

	return info->min_uv + val * info->step_uv;
}

static int ldo_set_enable(struct udevice *dev, bool enable)
{
	int ldo = dev->driver_data - 1;

	return _ldo_set_enable(dev->parent, ldo, enable);
}

static int ldo_set_suspend_enable(struct udevice *dev, bool enable)
{
	int ldo = dev->driver_data - 1;

	return _ldo_set_suspend_enable(dev->parent, ldo, enable);
}

static int ldo_get_suspend_enable(struct udevice *dev)
{
	int ldo = dev->driver_data - 1;

	return _ldo_get_suspend_enable(dev->parent, ldo);
}

static int ldo_get_enable(struct udevice *dev)
{
	int ldo = dev->driver_data - 1;

	return _ldo_get_enable(dev->parent, ldo);
}

static int switch_set_enable(struct udevice *dev, bool enable)
{
	struct rk8xx_priv *priv = dev_get_priv(dev->parent);
	int ret = 0, sw = dev->driver_data - 1;
	uint mask = 0;

	switch (priv->variant) {
	case RK808_ID:
		mask = 1 << (sw + 5);
		ret = pmic_clrsetbits(dev->parent, REG_DCDC_EN, mask,
				      enable ? mask : 0);
		break;
	case RK809_ID:
		mask = (1 << (sw + 2)) | (1 << (sw + 6));
		ret = pmic_clrsetbits(dev->parent, RK817_POWER_EN(3), mask,
				      enable ? mask : 0);
		break;
	case RK818_ID:
		mask = 1 << 6;
		ret = pmic_clrsetbits(dev->parent, REG_DCDC_EN, mask,
				      enable ? mask : 0);
		break;
	}

	debug("%s: switch%d, enable=%d, mask=0x%x\n",
	      __func__, sw + 1, enable, mask);

	return ret;
}

static int switch_get_enable(struct udevice *dev)
{
	struct rk8xx_priv *priv = dev_get_priv(dev->parent);
	int ret = 0, sw = dev->driver_data - 1;
	uint mask = 0;

	switch (priv->variant) {
	case RK808_ID:
		mask = 1 << (sw + 5);
		ret = pmic_reg_read(dev->parent, REG_DCDC_EN);
		break;
	case RK809_ID:
		mask = 1 << (sw + 2);
		ret = pmic_reg_read(dev->parent, RK817_POWER_EN(3));
		break;
	case RK818_ID:
		mask = 1 << 6;
		ret = pmic_reg_read(dev->parent, REG_DCDC_EN);
		break;
	}

	if (ret < 0)
		return ret;

	return ret & mask ? true : false;
}

static int switch_set_suspend_value(struct udevice *dev, int uvolt)
{
	return 0;
}

static int switch_get_suspend_value(struct udevice *dev)
{
	return 0;
}

static int switch_set_suspend_enable(struct udevice *dev, bool enable)
{
	struct rk8xx_priv *priv = dev_get_priv(dev->parent);
	int ret = 0, sw = dev->driver_data - 1;
	uint mask = 0;

	switch (priv->variant) {
	case RK808_ID:
		mask = 1 << (sw + 5);
		ret = pmic_clrsetbits(dev->parent, REG_SLEEP_SET_OFF1, mask,
				      enable ? 0 : mask);
		break;
	case RK809_ID:
		mask = 1 << (sw + 6);
		ret = pmic_clrsetbits(dev->parent, RK817_POWER_SLP_EN(0), mask,
				      enable ? mask : 0);
		break;
	case RK818_ID:
		mask = 1 << 6;
		ret = pmic_clrsetbits(dev->parent, REG_SLEEP_SET_OFF1, mask,
				      enable ? 0 : mask);
		break;
	}

	debug("%s: switch%d, enable=%d, mask=0x%x\n",
	      __func__, sw + 1, enable, mask);

	return ret;
}

static int switch_get_suspend_enable(struct udevice *dev)
{
	struct rk8xx_priv *priv = dev_get_priv(dev->parent);
	int val, ret = 0, sw = dev->driver_data - 1;
	uint mask = 0;

	switch (priv->variant) {
	case RK808_ID:
		mask = 1 << (sw + 5);
		val = pmic_reg_read(dev->parent, REG_SLEEP_SET_OFF1);
		if (val < 0)
			return val;
		ret = val & mask ? 0 : 1;
		break;
	case RK809_ID:
		mask = 1 << (sw + 6);
		val = pmic_reg_read(dev->parent, RK817_POWER_SLP_EN(0));
		if (val < 0)
			return val;
		ret = val & mask ? 1 : 0;
		break;
	case RK818_ID:
		mask = 1 << 6;
		val = pmic_reg_read(dev->parent, REG_SLEEP_SET_OFF1);
		if (val < 0)
			return val;
		ret = val & mask ? 0 : 1;
		break;
	}

	return ret;
}

/*
 * RK8xx switch does not need to set the voltage,
 * but if dts set regulator-min-microvolt/regulator-max-microvolt,
 * will cause regulator set value fail and not to enable this switch.
 * So add an empty function to return success.
 */
static int switch_get_value(struct udevice *dev)
{
	static const char * const supply_name_rk809[] = {
		"vcc9-supply",
		"vcc8-supply",
	};
	struct rk8xx_priv *priv = dev_get_priv(dev->parent);
	struct udevice *supply;
	int id = dev->driver_data - 1;

	if (!switch_get_enable(dev))
		return 0;

	if (priv->variant == RK809_ID) {
		if (!uclass_get_device_by_phandle(UCLASS_REGULATOR,
						  dev->parent,
						  supply_name_rk809[id],
						  &supply))
			return regulator_get_value(supply);
	}

	return 0;
}

static int switch_set_value(struct udevice *dev, int uvolt)
{
	return 0;
}

static int rk8xx_buck_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_BUCK;
	uc_pdata->mode_count = 0;

	return 0;
}

static int rk8xx_ldo_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_LDO;
	uc_pdata->mode_count = 0;

	return 0;
}

static int rk8xx_switch_probe(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata;

	uc_pdata = dev_get_uclass_plat(dev);

	uc_pdata->type = REGULATOR_TYPE_FIXED;
	uc_pdata->mode_count = 0;

	return 0;
}

static const struct dm_regulator_ops rk8xx_buck_ops = {
	.get_value  = buck_get_value,
	.set_value  = buck_set_value,
	.set_suspend_value = buck_set_suspend_value,
	.get_suspend_value = buck_get_suspend_value,
	.get_enable = buck_get_enable,
	.set_enable = buck_set_enable,
	.set_suspend_enable = buck_set_suspend_enable,
	.get_suspend_enable = buck_get_suspend_enable,
};

static const struct dm_regulator_ops rk8xx_ldo_ops = {
	.get_value  = ldo_get_value,
	.set_value  = ldo_set_value,
	.set_suspend_value = ldo_set_suspend_value,
	.get_suspend_value = ldo_get_suspend_value,
	.get_enable = ldo_get_enable,
	.set_enable = ldo_set_enable,
	.set_suspend_enable = ldo_set_suspend_enable,
	.get_suspend_enable = ldo_get_suspend_enable,
};

static const struct dm_regulator_ops rk8xx_switch_ops = {
	.get_value  = switch_get_value,
	.set_value  = switch_set_value,
	.get_enable = switch_get_enable,
	.set_enable = switch_set_enable,
	.set_suspend_enable = switch_set_suspend_enable,
	.get_suspend_enable = switch_get_suspend_enable,
	.set_suspend_value = switch_set_suspend_value,
	.get_suspend_value = switch_get_suspend_value,
};

U_BOOT_DRIVER(rk8xx_buck) = {
	.name = "rk8xx_buck",
	.id = UCLASS_REGULATOR,
	.ops = &rk8xx_buck_ops,
	.probe = rk8xx_buck_probe,
};

U_BOOT_DRIVER(rk8xx_ldo) = {
	.name = "rk8xx_ldo",
	.id = UCLASS_REGULATOR,
	.ops = &rk8xx_ldo_ops,
	.probe = rk8xx_ldo_probe,
};

U_BOOT_DRIVER(rk8xx_switch) = {
	.name = "rk8xx_switch",
	.id = UCLASS_REGULATOR,
	.ops = &rk8xx_switch_ops,
	.probe = rk8xx_switch_probe,
};
#endif

int rk8xx_spl_configure_buck(struct udevice *pmic, int buck, int uvolt)
{
	int ret;

	ret = _buck_set_value(pmic, buck, uvolt);
	if (ret)
		return ret;

	return _buck_set_enable(pmic, buck, true);
}

int rk818_spl_configure_usb_input_current(struct udevice *pmic, int current_ma)
{
	uint i;

	for (i = 0; i < ARRAY_SIZE(rk818_chrg_cur_input_array); i++)
		if (current_ma <= rk818_chrg_cur_input_array[i])
			break;

	return pmic_clrsetbits(pmic, REG_USB_CTRL, RK818_USB_ILIM_SEL_MASK, i);
}

int rk818_spl_configure_usb_chrg_shutdown(struct udevice *pmic, int uvolt)
{
	uint i;

	for (i = 0; i < ARRAY_SIZE(rk818_chrg_shutdown_vsel_array); i++)
		if (uvolt <= rk818_chrg_shutdown_vsel_array[i])
			break;

	return pmic_clrsetbits(pmic, REG_USB_CTRL, RK818_USB_CHG_SD_VSEL_MASK,
			       i);
}
