// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for IDT Versaclock 5/6
 *
 * Derived from code Copyright (C) 2017 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <dm/device_compat.h>
#include <log.h>
#include <linux/clk-provider.h>
#include <linux/kernel.h>
#include <linux/math64.h>

#include <dt-bindings/clock/versaclock.h>

/* VersaClock5 registers */
#define VC5_OTP_CONTROL				0x00

/* Factory-reserved register block */
#define VC5_RSVD_DEVICE_ID			0x01
#define VC5_RSVD_ADC_GAIN_7_0			0x02
#define VC5_RSVD_ADC_GAIN_15_8			0x03
#define VC5_RSVD_ADC_OFFSET_7_0			0x04
#define VC5_RSVD_ADC_OFFSET_15_8		0x05
#define VC5_RSVD_TEMPY				0x06
#define VC5_RSVD_OFFSET_TBIN			0x07
#define VC5_RSVD_GAIN				0x08
#define VC5_RSVD_TEST_NP			0x09
#define VC5_RSVD_UNUSED				0x0a
#define VC5_RSVD_BANDGAP_TRIM_UP		0x0b
#define VC5_RSVD_BANDGAP_TRIM_DN		0x0c
#define VC5_RSVD_CLK_R_12_CLK_AMP_4		0x0d
#define VC5_RSVD_CLK_R_34_CLK_AMP_4		0x0e
#define VC5_RSVD_CLK_AMP_123			0x0f

/* Configuration register block */
#define VC5_PRIM_SRC_SHDN			0x10
#define VC5_PRIM_SRC_SHDN_EN_XTAL		BIT(7)
#define VC5_PRIM_SRC_SHDN_EN_CLKIN		BIT(6)
#define VC5_PRIM_SRC_SHDN_EN_DOUBLE_XTAL_FREQ	BIT(3)
#define VC5_PRIM_SRC_SHDN_SP			BIT(1)
#define VC5_PRIM_SRC_SHDN_EN_GBL_SHDN		BIT(0)

#define VC5_VCO_BAND				0x11
#define VC5_XTAL_X1_LOAD_CAP			0x12
#define VC5_XTAL_X2_LOAD_CAP			0x13
#define VC5_REF_DIVIDER				0x15
#define VC5_REF_DIVIDER_SEL_PREDIV2		BIT(7)
#define VC5_REF_DIVIDER_REF_DIV(n)		((n) & 0x3f)

#define VC5_VCO_CTRL_AND_PREDIV			0x16
#define VC5_VCO_CTRL_AND_PREDIV_BYPASS_PREDIV	BIT(7)

#define VC5_FEEDBACK_INT_DIV			0x17
#define VC5_FEEDBACK_INT_DIV_BITS		0x18
#define VC5_FEEDBACK_FRAC_DIV(n)		(0x19 + (n))
#define VC5_RC_CONTROL0				0x1e
#define VC5_RC_CONTROL1				0x1f
/* Register 0x20 is factory reserved */

/* Output divider control for divider 1,2,3,4 */
#define VC5_OUT_DIV_CONTROL(idx)	(0x21 + ((idx) * 0x10))
#define VC5_OUT_DIV_CONTROL_RESET	BIT(7)
#define VC5_OUT_DIV_CONTROL_SELB_NORM	BIT(3)
#define VC5_OUT_DIV_CONTROL_SEL_EXT	BIT(2)
#define VC5_OUT_DIV_CONTROL_INT_MODE	BIT(1)
#define VC5_OUT_DIV_CONTROL_EN_FOD	BIT(0)

#define VC5_OUT_DIV_FRAC(idx, n)	(0x22 + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_FRAC4_OD_SCEE	BIT(1)

#define VC5_OUT_DIV_STEP_SPREAD(idx, n)	(0x26 + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_SPREAD_MOD(idx, n)	(0x29 + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_SKEW_INT(idx, n)	(0x2b + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_INT(idx, n)		(0x2d + ((idx) * 0x10) + (n))
#define VC5_OUT_DIV_SKEW_FRAC(idx)	(0x2f + ((idx) * 0x10))
/* Registers 0x30, 0x40, 0x50 are factory reserved */

/* Clock control register for clock 1,2 */
#define VC5_CLK_OUTPUT_CFG(idx, n)	(0x60 + ((idx) * 0x2) + (n))
#define VC5_CLK_OUTPUT_CFG0_CFG_SHIFT	5
#define VC5_CLK_OUTPUT_CFG0_CFG_MASK GENMASK(7, VC5_CLK_OUTPUT_CFG0_CFG_SHIFT)

#define VC5_CLK_OUTPUT_CFG0_CFG_LVPECL	(VC5_LVPECL)
#define VC5_CLK_OUTPUT_CFG0_CFG_CMOS		(VC5_CMOS)
#define VC5_CLK_OUTPUT_CFG0_CFG_HCSL33	(VC5_HCSL33)
#define VC5_CLK_OUTPUT_CFG0_CFG_LVDS		(VC5_LVDS)
#define VC5_CLK_OUTPUT_CFG0_CFG_CMOS2		(VC5_CMOS2)
#define VC5_CLK_OUTPUT_CFG0_CFG_CMOSD		(VC5_CMOSD)
#define VC5_CLK_OUTPUT_CFG0_CFG_HCSL25	(VC5_HCSL25)

#define VC5_CLK_OUTPUT_CFG0_PWR_SHIFT	3
#define VC5_CLK_OUTPUT_CFG0_PWR_MASK GENMASK(4, VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_PWR_18	(0 << VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_PWR_25	(2 << VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_PWR_33	(3 << VC5_CLK_OUTPUT_CFG0_PWR_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT	0
#define VC5_CLK_OUTPUT_CFG0_SLEW_MASK GENMASK(1, VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_80	(0 << VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_85	(1 << VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_90	(2 << VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG0_SLEW_100	(3 << VC5_CLK_OUTPUT_CFG0_SLEW_SHIFT)
#define VC5_CLK_OUTPUT_CFG1_EN_CLKBUF	BIT(0)

#define VC5_CLK_OE_SHDN				0x68
#define VC5_CLK_OS_SHDN				0x69

#define VC5_GLOBAL_REGISTER			0x76
#define VC5_GLOBAL_REGISTER_GLOBAL_RESET	BIT(5)

/* PLL/VCO runs between 2.5 GHz and 3.0 GHz */
#define VC5_PLL_VCO_MIN				2500000000UL
#define VC5_PLL_VCO_MAX				3000000000UL

/* VC5 Input mux settings */
#define VC5_MUX_IN_XIN		BIT(0)
#define VC5_MUX_IN_CLKIN	BIT(1)

/* Maximum number of clk_out supported by this driver */
#define VC5_MAX_CLK_OUT_NUM	5

/* Maximum number of FODs supported by this driver */
#define VC5_MAX_FOD_NUM	4

/* flags to describe chip features */
/* chip has built-in oscilator */
#define VC5_HAS_INTERNAL_XTAL	BIT(0)
/* chip has PFD requency doubler */
#define VC5_HAS_PFD_FREQ_DBL	BIT(1)

/* Supported IDT VC5 models. */
enum vc5_model {
	IDT_VC5_5P49V5923,
	IDT_VC5_5P49V5925,
	IDT_VC5_5P49V5933,
	IDT_VC5_5P49V5935,
	IDT_VC6_5P49V6901,
	IDT_VC6_5P49V6965,
};

/* Structure to describe features of a particular VC5 model */
struct vc5_chip_info {
	const enum vc5_model	model;
	const unsigned int	clk_fod_cnt;
	const unsigned int	clk_out_cnt;
	const u32		flags;
};

struct vc5_driver_data;

struct vc5_hw_data {
	struct clk		hw;
	struct vc5_driver_data	*vc5;
	u32			div_int;
	u32			div_frc;
	unsigned int		num;
};

struct vc5_out_data {
	struct clk		hw;
	struct vc5_driver_data	*vc5;
	unsigned int		num;
	unsigned int		clk_output_cfg0;
	unsigned int		clk_output_cfg0_mask;
};

struct vc5_driver_data {
	struct udevice		*i2c;
	const struct vc5_chip_info	*chip_info;

	struct clk		*pin_xin;
	struct clk		*pin_clkin;
	unsigned char		clk_mux_ins;
	struct clk		clk_mux;
	struct clk		clk_mul;
	struct clk		clk_pfd;
	struct vc5_hw_data	clk_pll;
	struct vc5_hw_data	clk_fod[VC5_MAX_FOD_NUM];
	struct vc5_out_data	clk_out[VC5_MAX_CLK_OUT_NUM];
};

static const struct vc5_chip_info idt_5p49v5923_info = {
	.model = IDT_VC5_5P49V5923,
	.clk_fod_cnt = 2,
	.clk_out_cnt = 3,
	.flags = 0,
};

static const struct vc5_chip_info idt_5p49v5925_info = {
	.model = IDT_VC5_5P49V5925,
	.clk_fod_cnt = 4,
	.clk_out_cnt = 5,
	.flags = 0,
};

static const struct vc5_chip_info idt_5p49v5933_info = {
	.model = IDT_VC5_5P49V5933,
	.clk_fod_cnt = 2,
	.clk_out_cnt = 3,
	.flags = VC5_HAS_INTERNAL_XTAL,
};

static const struct vc5_chip_info idt_5p49v5935_info = {
	.model = IDT_VC5_5P49V5935,
	.clk_fod_cnt = 4,
	.clk_out_cnt = 5,
	.flags = VC5_HAS_INTERNAL_XTAL,
};

static const struct vc5_chip_info idt_5p49v6901_info = {
	.model = IDT_VC6_5P49V6901,
	.clk_fod_cnt = 4,
	.clk_out_cnt = 5,
	.flags = VC5_HAS_PFD_FREQ_DBL,
};

static const struct vc5_chip_info idt_5p49v6965_info = {
	.model = IDT_VC6_5P49V6965,
	.clk_fod_cnt = 4,
	.clk_out_cnt = 5,
	.flags = 0,
};

static int vc5_update_bits(struct udevice *dev, unsigned int reg, unsigned int mask,
			   unsigned int src)
{
	int ret;
	unsigned char cache;

	ret = dm_i2c_read(dev, reg, &cache, 1);
	if (ret < 0)
		return ret;

	cache &= ~mask;
	cache |= mask & src;
	ret = dm_i2c_write(dev, reg, (uchar *)&cache, 1);

	return ret;
}

static unsigned long vc5_mux_get_rate(struct clk *hw)
{
	return clk_get_rate(clk_get_parent(hw));
}

static int vc5_mux_set_parent(struct clk *hw, unsigned char index)
{
	struct vc5_driver_data *vc5 = container_of(hw, struct vc5_driver_data, clk_mux);
	const u8 mask = VC5_PRIM_SRC_SHDN_EN_XTAL | VC5_PRIM_SRC_SHDN_EN_CLKIN;
	u8 src;

	if (index > 1 || !vc5->clk_mux_ins)
		return -EINVAL;

	if (vc5->clk_mux_ins == (VC5_MUX_IN_CLKIN | VC5_MUX_IN_XIN)) {
		if (index == 0)
			src = VC5_PRIM_SRC_SHDN_EN_XTAL;
		if (index == 1)
			src = VC5_PRIM_SRC_SHDN_EN_CLKIN;
	} else {
		if (index != 0)
			return -EINVAL;

		if (vc5->clk_mux_ins == VC5_MUX_IN_XIN)
			src = VC5_PRIM_SRC_SHDN_EN_XTAL;
		else if (vc5->clk_mux_ins == VC5_MUX_IN_CLKIN)
			src = VC5_PRIM_SRC_SHDN_EN_CLKIN;
		else /* Invalid; should have been caught by vc5_probe() */
			return -EINVAL;
	}

	return vc5_update_bits(vc5->i2c, VC5_PRIM_SRC_SHDN, mask, src);
}

static const struct clk_ops vc5_mux_ops = {
	.get_rate	= vc5_mux_get_rate,
};

static unsigned long vc5_pfd_round_rate(struct clk *hw, unsigned long rate)
{
	struct clk *clk_parent = clk_get_parent(hw);
	unsigned long parent_rate = clk_get_rate(clk_parent);
	unsigned long idiv;

	/* PLL cannot operate with input clock above 50 MHz. */
	if (rate > 50000000)
		return -EINVAL;

	/* CLKIN within range of PLL input, feed directly to PLL. */
	if (parent_rate <= 50000000)
		return parent_rate;

	idiv = DIV_ROUND_UP(parent_rate, rate);
	if (idiv > 127)
		return -EINVAL;

	return parent_rate / idiv;
}

static unsigned long vc5_pfd_recalc_rate(struct clk *hw)
{
	struct vc5_driver_data *vc5 =
		container_of(hw, struct vc5_driver_data, clk_pfd);
	unsigned int prediv, div;
	struct clk *clk_parent = clk_get_parent(hw);
	unsigned long parent_rate = clk_get_rate(clk_parent);

	dm_i2c_read(vc5->i2c, VC5_VCO_CTRL_AND_PREDIV, (uchar *)&prediv, 1);

	/* The bypass_prediv is set, PLL fed from Ref_in directly. */
	if (prediv & VC5_VCO_CTRL_AND_PREDIV_BYPASS_PREDIV)
		return parent_rate;

	dm_i2c_read(vc5->i2c, VC5_REF_DIVIDER, (uchar *)&div, 1);

	/* The Sel_prediv2 is set, PLL fed from prediv2 (Ref_in / 2) */
	if (div & VC5_REF_DIVIDER_SEL_PREDIV2)
		return parent_rate / 2;
	else
		return parent_rate / VC5_REF_DIVIDER_REF_DIV(div);
}

static unsigned long vc5_pfd_set_rate(struct clk *hw, unsigned long rate)
{
	struct vc5_driver_data *vc5 =
		container_of(hw, struct vc5_driver_data, clk_pfd);
	unsigned long idiv;
	u8 div;
	struct clk *clk_parent = clk_get_parent(hw);
	unsigned long parent_rate = clk_get_rate(clk_parent);

	/* CLKIN within range of PLL input, feed directly to PLL. */
	if (parent_rate <= 50000000) {
		vc5_update_bits(vc5->i2c, VC5_VCO_CTRL_AND_PREDIV,
				VC5_VCO_CTRL_AND_PREDIV_BYPASS_PREDIV,
				VC5_VCO_CTRL_AND_PREDIV_BYPASS_PREDIV);
		vc5_update_bits(vc5->i2c, VC5_REF_DIVIDER, 0xff, 0x00);
		return 0;
	}

	idiv = DIV_ROUND_UP(parent_rate, rate);

	/* We have dedicated div-2 predivider. */
	if (idiv == 2)
		div = VC5_REF_DIVIDER_SEL_PREDIV2;
	else
		div = VC5_REF_DIVIDER_REF_DIV(idiv);

	vc5_update_bits(vc5->i2c, VC5_REF_DIVIDER, 0xff, div);
	vc5_update_bits(vc5->i2c, VC5_VCO_CTRL_AND_PREDIV,
			VC5_VCO_CTRL_AND_PREDIV_BYPASS_PREDIV, 0);

	return 0;
}

static const struct clk_ops vc5_pfd_ops = {
	.round_rate	= vc5_pfd_round_rate,
	.get_rate	= vc5_pfd_recalc_rate,
	.set_rate	= vc5_pfd_set_rate,
};

/*
 * VersaClock5 PLL/VCO
 */
static unsigned long vc5_pll_recalc_rate(struct clk *hw)
{
	struct vc5_hw_data *hwdata = container_of(hw, struct vc5_hw_data, hw);
	struct vc5_driver_data *vc = hwdata->vc5;
	struct clk *clk_parent = clk_get_parent(hw);
	unsigned long parent_rate = clk_get_rate(clk_parent);
	u32 div_int, div_frc;
	u8 fb[5];

	dm_i2c_read(vc->i2c, VC5_FEEDBACK_INT_DIV, fb, 5);

	div_int = (fb[0] << 4) | (fb[1] >> 4);
	div_frc = (fb[2] << 16) | (fb[3] << 8) | fb[4];

	/* The PLL divider has 12 integer bits and 24 fractional bits */
	return (parent_rate * div_int) + ((parent_rate * div_frc) >> 24);
}

static unsigned long vc5_pll_round_rate(struct clk *hw, unsigned long rate)
{
	struct clk *clk_parent = clk_get_parent(hw);
	unsigned long parent_rate = clk_get_rate(clk_parent);
	struct vc5_hw_data *hwdata = container_of(hw, struct vc5_hw_data, hw);
	u32 div_int;
	u64 div_frc;

	if (rate < VC5_PLL_VCO_MIN)
		rate = VC5_PLL_VCO_MIN;
	if (rate > VC5_PLL_VCO_MAX)
		rate = VC5_PLL_VCO_MAX;

	/* Determine integer part, which is 12 bit wide */
	div_int = rate / parent_rate;
	if (div_int > 0xfff)
		rate = parent_rate * 0xfff;

	/* Determine best fractional part, which is 24 bit wide */
	div_frc = rate % parent_rate;
	div_frc *= BIT(24) - 1;
	do_div(div_frc, parent_rate);

	hwdata->div_int = div_int;
	hwdata->div_frc = (u32)div_frc;

	return (parent_rate * div_int) + ((parent_rate * div_frc) >> 24);
}

static unsigned long vc5_pll_set_rate(struct clk *hw, unsigned long rate)
{
	struct vc5_hw_data *hwdata = container_of(hw, struct vc5_hw_data, hw);
	struct vc5_driver_data *vc5 = hwdata->vc5;
	u8 fb[5];

	fb[0] = hwdata->div_int >> 4;
	fb[1] = hwdata->div_int << 4;
	fb[2] = hwdata->div_frc >> 16;
	fb[3] = hwdata->div_frc >> 8;
	fb[4] = hwdata->div_frc;

	return dm_i2c_write(vc5->i2c, VC5_FEEDBACK_INT_DIV, fb, 5);
}

static const struct clk_ops vc5_pll_ops = {
	.round_rate	= vc5_pll_round_rate,
	.get_rate	= vc5_pll_recalc_rate,
	.set_rate	= vc5_pll_set_rate,
};

static unsigned long vc5_fod_recalc_rate(struct clk *hw)
{
	struct vc5_hw_data *hwdata = container_of(hw, struct vc5_hw_data, hw);
	struct vc5_driver_data *vc = hwdata->vc5;
	struct clk *parent = &vc->clk_pll.hw;
	unsigned long parent_rate =  vc5_pll_recalc_rate(parent);

	/* VCO frequency is divided by two before entering FOD */
	u32 f_in = parent_rate / 2;
	u32 div_int, div_frc;
	u8 od_int[2];
	u8 od_frc[4];

	dm_i2c_read(vc->i2c, VC5_OUT_DIV_INT(hwdata->num, 0), od_int, 2);
	dm_i2c_read(vc->i2c, VC5_OUT_DIV_FRAC(hwdata->num, 0), od_frc, 4);

	div_int = (od_int[0] << 4) | (od_int[1] >> 4);
	div_frc = (od_frc[0] << 22) | (od_frc[1] << 14) |
		  (od_frc[2] << 6) | (od_frc[3] >> 2);

	/* Avoid division by zero if the output is not configured. */
	if (div_int == 0 && div_frc == 0)
		return 0;

	/* The PLL divider has 12 integer bits and 30 fractional bits */
	return div64_u64((u64)f_in << 24ULL, ((u64)div_int << 24ULL) + div_frc);
}

static unsigned long vc5_fod_round_rate(struct clk *hw, unsigned long rate)
{
	struct vc5_hw_data *hwdata = container_of(hw, struct vc5_hw_data, hw);
	struct vc5_driver_data *vc = hwdata->vc5;
	struct clk *parent = &vc->clk_pll.hw;
	unsigned long parent_rate =  vc5_pll_recalc_rate(parent);

	/* VCO frequency is divided by two before entering FOD */
	u32 f_in = parent_rate / 2;
	u32 div_int;
	u64 div_frc;

	/* Determine integer part, which is 12 bit wide */
	div_int = f_in / rate;

	/*
	 * WARNING: The clock chip does not output signal if the integer part
	 *          of the divider is 0xfff and fractional part is non-zero.
	 *          Clamp the divider at 0xffe to keep the code simple.
	 */
	if (div_int > 0xffe) {
		div_int = 0xffe;
		rate = f_in / div_int;
	}

	/* Determine best fractional part, which is 30 bit wide */
	div_frc = f_in % rate;
	div_frc <<= 24;
	do_div(div_frc, rate);

	hwdata->div_int = div_int;
	hwdata->div_frc = (u32)div_frc;

	return div64_u64((u64)f_in << 24ULL, ((u64)div_int << 24ULL) + div_frc);
}

static unsigned long vc5_fod_set_rate(struct clk *hw, unsigned long rate)
{
	struct vc5_hw_data *hwdata = container_of(hw, struct vc5_hw_data, hw);
	struct vc5_driver_data *vc5 = hwdata->vc5;

	u8 data[14] = {
		hwdata->div_frc >> 22, hwdata->div_frc >> 14,
		hwdata->div_frc >> 6, hwdata->div_frc << 2,
		0, 0, 0, 0, 0,
		0, 0,
		hwdata->div_int >> 4, hwdata->div_int << 4,
		0
	};

	dm_i2c_write(vc5->i2c, VC5_OUT_DIV_FRAC(hwdata->num, 0), data, 14);

	/*
	 * Toggle magic bit in undocumented register for unknown reason.
	 * This is what the IDT timing commander tool does and the chip
	 * datasheet somewhat implies this is needed, but the register
	 * and the bit is not documented.
	 */
	vc5_update_bits(vc5->i2c, VC5_GLOBAL_REGISTER,
			VC5_GLOBAL_REGISTER_GLOBAL_RESET, 0);
	vc5_update_bits(vc5->i2c, VC5_GLOBAL_REGISTER,
			VC5_GLOBAL_REGISTER_GLOBAL_RESET,
			VC5_GLOBAL_REGISTER_GLOBAL_RESET);

	return 0;
}

static const struct clk_ops vc5_fod_ops = {
	.round_rate	= vc5_fod_round_rate,
	.get_rate	= vc5_fod_recalc_rate,
	.set_rate	= vc5_fod_set_rate,
};

static int vc5_clk_out_prepare(struct clk *hw)
{
	struct udevice *dev;
	struct vc5_driver_data *vc5;
	struct vc5_out_data *hwdata;

	const u8 mask = VC5_OUT_DIV_CONTROL_SELB_NORM |
			VC5_OUT_DIV_CONTROL_SEL_EXT |
			VC5_OUT_DIV_CONTROL_EN_FOD;
	unsigned int src;
	int ret;

	uclass_get_device_by_name(UCLASS_CLK, clk_hw_get_name(hw), &dev);
	vc5 = dev_get_priv(dev);
	hwdata = &vc5->clk_out[hw->id];

	/*
	 * If the input mux is disabled, enable it first and
	 * select source from matching FOD.
	 */

	dm_i2c_read(vc5->i2c, VC5_OUT_DIV_CONTROL(hwdata->num), (uchar *)&src, 1);

	if ((src & mask) == 0) {
		src = VC5_OUT_DIV_CONTROL_RESET | VC5_OUT_DIV_CONTROL_EN_FOD;
		ret = vc5_update_bits(vc5->i2c,
				      VC5_OUT_DIV_CONTROL(hwdata->num),
				      mask | VC5_OUT_DIV_CONTROL_RESET, src);
		if (ret)
			return ret;
	}

	/* Enable the clock buffer */
	vc5_update_bits(vc5->i2c, VC5_CLK_OUTPUT_CFG(hwdata->num, 1),
			VC5_CLK_OUTPUT_CFG1_EN_CLKBUF,
			VC5_CLK_OUTPUT_CFG1_EN_CLKBUF);
	if (hwdata->clk_output_cfg0_mask) {
		vc5_update_bits(vc5->i2c, VC5_CLK_OUTPUT_CFG(hwdata->num, 0),
				hwdata->clk_output_cfg0_mask,
				hwdata->clk_output_cfg0);
	}

	return 0;
}

static int vc5_clk_out_unprepare(struct clk *hw)
{
	struct udevice *dev;
	struct vc5_driver_data *vc5;
	struct vc5_out_data *hwdata;
	int ret;

	uclass_get_device_by_name(UCLASS_CLK, clk_hw_get_name(hw), &dev);
	vc5 = dev_get_priv(dev);
	hwdata = &vc5->clk_out[hw->id];

	/* Disable the clock buffer */
	ret = vc5_update_bits(vc5->i2c, VC5_CLK_OUTPUT_CFG(hwdata->num, 1),
			      VC5_CLK_OUTPUT_CFG1_EN_CLKBUF, 0);

	return ret;
}

static int vc5_clk_out_set_parent(struct vc5_driver_data *vc, u8 num, u8 index)
{
	const u8 mask = VC5_OUT_DIV_CONTROL_RESET |
			VC5_OUT_DIV_CONTROL_SELB_NORM |
			VC5_OUT_DIV_CONTROL_SEL_EXT |
			VC5_OUT_DIV_CONTROL_EN_FOD;
	const u8 extclk = VC5_OUT_DIV_CONTROL_SELB_NORM |
			  VC5_OUT_DIV_CONTROL_SEL_EXT;
	u8 src = VC5_OUT_DIV_CONTROL_RESET;

	if (index == 0)
		src |= VC5_OUT_DIV_CONTROL_EN_FOD;
	else
		src |= extclk;

	return vc5_update_bits(vc->i2c, VC5_OUT_DIV_CONTROL(num), mask, src);
}

static unsigned long vc5_clk_out_set_rate(struct clk *hw, unsigned long rate)
{
	struct udevice *dev;
	struct vc5_driver_data *vc;
	struct clk *parent;

	uclass_get_device_by_name(UCLASS_CLK, clk_hw_get_name(hw), &dev);
	vc = dev_get_priv(dev);
	parent = clk_get_parent(&vc->clk_out[hw->id].hw);

	/* setting the output rate really means setting the parent FOD rate */
	return clk_set_rate(parent, clk_round_rate(parent, rate));
}

static unsigned long vc5_clk_out_get_rate(struct clk *hw)
{
	return clk_get_parent_rate(hw);
}

static const struct clk_ops vc5_clk_out_ops = {
	.enable	= vc5_clk_out_prepare,
	.disable	= vc5_clk_out_unprepare,
	.set_rate	= vc5_clk_out_set_rate,
	.get_rate	= vc5_clk_out_get_rate,
};

static const struct clk_ops vc5_clk_out_sel_ops = {
	.enable	= vc5_clk_out_prepare,
	.disable	= vc5_clk_out_unprepare,
	.get_rate	= vc5_clk_out_get_rate,
};

static const struct clk_ops vc5_clk_ops = {
	.enable	= vc5_clk_out_prepare,
	.disable	= vc5_clk_out_unprepare,
	.set_rate	= vc5_clk_out_set_rate,
	.get_rate	= vc5_clk_out_get_rate,
};

static int vc5_map_index_to_output(const enum vc5_model model,
				   const unsigned int n)
{
	switch (model) {
	case IDT_VC5_5P49V5933:
		return (n == 0) ? 0 : 3;
	case IDT_VC5_5P49V5923:
	case IDT_VC5_5P49V5925:
	case IDT_VC5_5P49V5935:
	case IDT_VC6_5P49V6901:
	case IDT_VC6_5P49V6965:
	default:
		return n;
	}
}

static int vc5_update_mode(ofnode np_output,
			   struct vc5_out_data *clk_out)
{
	u32 value;

	if (!ofnode_read_u32(np_output, "idt,mode", &value)) {
		clk_out->clk_output_cfg0_mask |= VC5_CLK_OUTPUT_CFG0_CFG_MASK;
		switch (value) {
		case VC5_CLK_OUTPUT_CFG0_CFG_LVPECL:
		case VC5_CLK_OUTPUT_CFG0_CFG_CMOS:
		case VC5_CLK_OUTPUT_CFG0_CFG_HCSL33:
		case VC5_CLK_OUTPUT_CFG0_CFG_LVDS:
		case VC5_CLK_OUTPUT_CFG0_CFG_CMOS2:
		case VC5_CLK_OUTPUT_CFG0_CFG_CMOSD:
		case VC5_CLK_OUTPUT_CFG0_CFG_HCSL25:
			clk_out->clk_output_cfg0 |=
			    value << VC5_CLK_OUTPUT_CFG0_CFG_SHIFT;
			break;
		default:
			return -EINVAL;
		}
	}

	return 0;
}

static int vc5_update_power(ofnode np_output, struct vc5_out_data *clk_out)
{
	u32 value;

	if (!ofnode_read_u32(np_output, "idt,voltage-microvolt", &value)) {
		clk_out->clk_output_cfg0_mask |= VC5_CLK_OUTPUT_CFG0_PWR_MASK;
		switch (value) {
		case 1800000:
			clk_out->clk_output_cfg0 |= VC5_CLK_OUTPUT_CFG0_PWR_18;
			break;
		case 2500000:
			clk_out->clk_output_cfg0 |= VC5_CLK_OUTPUT_CFG0_PWR_25;
			break;
		case 3300000:
			clk_out->clk_output_cfg0 |= VC5_CLK_OUTPUT_CFG0_PWR_33;
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

static int vc5_map_cap_value(u32 femtofarads)
{
	int mapped_value;

	/*
	 * The datasheet explicitly states 9000 - 25000 with 0.5pF
	 * steps, but the Programmer's guide shows the steps are 0.430pF.
	 * After getting feedback from Renesas, the .5pF steps were the
	 * goal, but 430nF was the actual values.
	 * Because of this, the actual range goes to 22760 instead of 25000
	 */
	if (femtofarads < 9000 || femtofarads > 22760)
		return -EINVAL;

	/*
	 * The Programmer's guide shows XTAL[5:0] but in reality,
	 * XTAL[0] and XTAL[1] are both LSB which makes the math
	 * strange.  With clarfication from Renesas, setting the
	 * values should be simpler by ignoring XTAL[0]
	 */
	mapped_value = DIV_ROUND_CLOSEST(femtofarads - 9000, 430);

	/*
	 * Since the calculation ignores XTAL[0], there is one
	 * special case where mapped_value = 32.  In reality, this means
	 * the real mapped value should be 111111b.  In other cases,
	 * the mapped_value needs to be shifted 1 to the left.
	 */
	if (mapped_value > 31)
		mapped_value = 0x3f;
	else
		mapped_value <<= 1;

	return mapped_value;
}

static int vc5_update_cap_load(ofnode node, struct vc5_driver_data *vc5)
{
	u32 value;
	int mapped_value;

	if (!ofnode_read_u32(node, "idt,xtal-load-femtofarads", &value)) {
		mapped_value = vc5_map_cap_value(value);

		if (mapped_value < 0)
			return mapped_value;

		/*
		 * The mapped_value is really the high 6 bits of
		 * VC5_XTAL_X1_LOAD_CAP and VC5_XTAL_X2_LOAD_CAP, so
		 * shift the value 2 places.
		 */
		vc5_update_bits(vc5->i2c, VC5_XTAL_X1_LOAD_CAP, ~0x03, mapped_value << 2);
		vc5_update_bits(vc5->i2c, VC5_XTAL_X2_LOAD_CAP, ~0x03, mapped_value << 2);
	}

	return 0;
}

static int vc5_update_slew(ofnode np_output, struct vc5_out_data *clk_out)
{
	u32 value;

	if (!ofnode_read_u32(np_output, "idt,slew-percent", &value)) {
		clk_out->clk_output_cfg0_mask |= VC5_CLK_OUTPUT_CFG0_SLEW_MASK;

		switch (value) {
		case 80:
			clk_out->clk_output_cfg0 |= VC5_CLK_OUTPUT_CFG0_SLEW_80;
			break;
		case 85:
			clk_out->clk_output_cfg0 |= VC5_CLK_OUTPUT_CFG0_SLEW_85;
			break;
		case 90:
			clk_out->clk_output_cfg0 |= VC5_CLK_OUTPUT_CFG0_SLEW_90;
			break;
		case 100:
			clk_out->clk_output_cfg0 |=
			    VC5_CLK_OUTPUT_CFG0_SLEW_100;
			break;
		default:
			return -EINVAL;
		}
	}
	return 0;
}

static int vc5_get_output_config(struct udevice *dev,
				 struct vc5_out_data *clk_out)
{
	ofnode np_output;
	char child_name[5];
	int ret = 0;

	sprintf(child_name, "OUT%d", clk_out->num + 1);

	np_output = dev_read_subnode(dev, child_name);

	if (!ofnode_valid(np_output)) {
		dev_dbg(dev, "Invalid clock output configuration OUT%d\n",
			clk_out->num + 1);
		return 0;
	}

	ret = vc5_update_mode(np_output, clk_out);
	if (ret)
		return ret;

	ret = vc5_update_power(np_output, clk_out);
	if (ret)
		return ret;

	ret = vc5_update_slew(np_output, clk_out);

	return ret;
}

static char *versaclock_get_name(const char *dev_name, const char *clk_name, int index)
{
	int length;
	char *buf;

	if (index < 0)
		length = snprintf(NULL, 0, "%s.%s", dev_name, clk_name) + 1;
	else
		length = snprintf(NULL, 0, "%s.%s%d", dev_name, clk_name, index) + 1;

	buf = malloc(length);
	if (!buf)
		ERR_PTR(-ENOMEM);

	if (index < 0)
		snprintf(buf, length, "%s.%s", dev_name, clk_name);
	else
		snprintf(buf, length, "%s.%s%d", dev_name, clk_name, index);

	return buf;
}

int versaclock_probe(struct udevice *dev)
{
	struct vc5_driver_data *vc5 = dev_get_priv(dev);
	struct vc5_chip_info *chip = (void *)dev_get_driver_data(dev);
	unsigned int n, idx = 0;
	char *mux_name, *pfd_name, *pll_name, *outsel_name;
	char *out_name[VC5_MAX_CLK_OUT_NUM];
	char *fod_name[VC5_MAX_FOD_NUM];
	int ret;
	u64 val;

	val = (u64)dev_read_addr_ptr(dev);
	ret = i2c_get_chip(dev->parent, val, 1, &vc5->i2c);

	if (ret) {
		dev_dbg(dev, "I2C probe failed.\n");
		return ret;
	}

	vc5->chip_info = chip;
	vc5->pin_xin = devm_clk_get(dev, "xin");

	if (IS_ERR(vc5->pin_xin))
		dev_dbg(dev, "failed to get xin clock\n");

	ret = clk_enable(vc5->pin_xin);
	if (ret)
		dev_dbg(dev, "failed to enable XIN clock\n");

	vc5->pin_clkin = devm_clk_get(dev, "clkin");

	/* Register clock input mux */
	if (!IS_ERR(vc5->pin_xin)) {
		vc5->clk_mux_ins |= VC5_MUX_IN_XIN;
	} else if (vc5->chip_info->flags & VC5_HAS_INTERNAL_XTAL) {
		if (IS_ERR(vc5->pin_xin))
			return PTR_ERR(vc5->pin_xin);
		vc5->clk_mux_ins |= VC5_MUX_IN_XIN;
	}

	mux_name = versaclock_get_name(dev->name, "mux", -1);
	if (IS_ERR(mux_name))
		return PTR_ERR(mux_name);

	clk_register(&vc5->clk_mux, "versaclock-mux", mux_name, vc5->pin_xin->dev->name);

	if (!IS_ERR(vc5->pin_xin))
		vc5_mux_set_parent(&vc5->clk_mux, 1);
	else
		vc5_mux_set_parent(&vc5->clk_mux, 0);

	/* Configure Optional Loading Capacitance for external XTAL */
	if (!(vc5->chip_info->flags & VC5_HAS_INTERNAL_XTAL)) {
		ret = vc5_update_cap_load(dev_ofnode(dev), vc5);
		if (ret)
			dev_dbg(dev, "failed to vc5_update_cap_load\n");
	}

	/* Register PFD */
	pfd_name = versaclock_get_name(dev->name, "pfd", -1);
	if (IS_ERR(pfd_name)) {
		ret = PTR_ERR(pfd_name);
		goto free_mux;
	}

	ret = clk_register(&vc5->clk_pfd, "versaclock-pfd", pfd_name, vc5->clk_mux.dev->name);
	if (ret)
		goto free_pfd;

	/* Register PLL */
	vc5->clk_pll.num = 0;
	vc5->clk_pll.vc5 = vc5;
	pll_name = versaclock_get_name(dev->name, "pll", -1);
	if (IS_ERR(pll_name)) {
		ret = PTR_ERR(pll_name);
		goto free_pfd;
	}

	ret = clk_register(&vc5->clk_pll.hw, "versaclock-pll", pll_name, vc5->clk_pfd.dev->name);
	if (ret)
		goto free_pll;

	/* Register FODs */
	for (n = 0; n < vc5->chip_info->clk_fod_cnt; n++) {
		fod_name[n] = versaclock_get_name(dev->name, "fod", n);
		if (IS_ERR(pll_name)) {
			ret = PTR_ERR(fod_name[n]);
			goto free_fod;
		}
		idx = vc5_map_index_to_output(vc5->chip_info->model, n);
		vc5->clk_fod[n].num = idx;
		vc5->clk_fod[n].vc5 = vc5;
		ret = clk_register(&vc5->clk_fod[n].hw, "versaclock-fod", fod_name[n],
				   vc5->clk_pll.hw.dev->name);
		if (ret)
			goto free_fod;
	}

	/* Register MUX-connected OUT0_I2C_SELB output */
	vc5->clk_out[0].num = idx;
	vc5->clk_out[0].vc5 = vc5;
	outsel_name = versaclock_get_name(dev->name, "out0_sel_i2cb", -1);
	if (IS_ERR(outsel_name)) {
		ret = PTR_ERR(outsel_name);
		goto free_fod;
	};

	ret = clk_register(&vc5->clk_out[0].hw, "versaclock-outsel",  outsel_name,
			   vc5->clk_mux.dev->name);
	if (ret)
		goto free_selb;

	/* Register FOD-connected OUTx outputs */
	for (n = 1; n < vc5->chip_info->clk_out_cnt; n++) {
		idx = vc5_map_index_to_output(vc5->chip_info->model, n - 1);
		out_name[n] = versaclock_get_name(dev->name, "out", n);
		if (IS_ERR(out_name[n])) {
			ret = PTR_ERR(out_name[n]);
			goto free_selb;
		}
		vc5->clk_out[n].num = idx;
		vc5->clk_out[n].vc5 = vc5;
		ret = clk_register(&vc5->clk_out[n].hw, "versaclock-out", out_name[n],
				   vc5->clk_fod[idx].hw.dev->name);
		if (ret)
			goto free_out;
		vc5_clk_out_set_parent(vc5, idx, 0);

		/* Fetch Clock Output configuration from DT (if specified) */
		ret = vc5_get_output_config(dev, &vc5->clk_out[n]);
		if (ret) {
			dev_dbg(dev, "failed to vc5_get_output_config()\n");
			goto free_out;
		}
	}

	return 0;

free_out:
	for (n = 1; n < vc5->chip_info->clk_out_cnt; n++)
		free(out_name[n]);
free_selb:
	free(outsel_name);
free_fod:
	for (n = 0; n < vc5->chip_info->clk_fod_cnt; n++)
		free(fod_name[n]);
free_pll:
	free(pll_name);
free_pfd:
	free(pfd_name);
free_mux:
	free(mux_name);

	return ret;
}

static const struct udevice_id versaclock_ids[] = {
	{ .compatible = "idt,5p49v5923", .data = (ulong)&idt_5p49v5923_info },
	{ .compatible = "idt,5p49v5925", .data = (ulong)&idt_5p49v5925_info },
	{ .compatible = "idt,5p49v5933", .data = (ulong)&idt_5p49v5933_info },
	{ .compatible = "idt,5p49v5935", .data = (ulong)&idt_5p49v5935_info },
	{ .compatible = "idt,5p49v6901", .data = (ulong)&idt_5p49v6901_info },
	{ .compatible = "idt,5p49v6965", .data = (ulong)&idt_5p49v6965_info },
	{},
};

U_BOOT_DRIVER(versaclock) = {
	.name           = "versaclock",
	.id             = UCLASS_CLK,
	.ops		= &vc5_clk_ops,
	.of_match       = versaclock_ids,
	.probe		= versaclock_probe,
	.priv_auto	= sizeof(struct vc5_driver_data),
};

U_BOOT_DRIVER(versaclock_mux) = {
	.name           = "versaclock-mux",
	.id             = UCLASS_CLK,
	.ops		= &vc5_mux_ops,
};

U_BOOT_DRIVER(versaclock_pfd) = {
	.name           = "versaclock-pfd",
	.id             = UCLASS_CLK,
	.ops		= &vc5_pfd_ops,
};

U_BOOT_DRIVER(versaclock_pll) = {
	.name           = "versaclock-pll",
	.id             = UCLASS_CLK,
	.ops		= &vc5_pll_ops,
};

U_BOOT_DRIVER(versaclock_fod) = {
	.name           = "versaclock-fod",
	.id             = UCLASS_CLK,
	.ops		= &vc5_fod_ops,
};

U_BOOT_DRIVER(versaclock_out) = {
	.name           = "versaclock-out",
	.id             = UCLASS_CLK,
	.ops		= &vc5_clk_out_ops,
};

U_BOOT_DRIVER(versaclock_outsel) = {
	.name           = "versaclock-outsel",
	.id             = UCLASS_CLK,
	.ops		= &vc5_clk_out_sel_ops,
};
