/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef _ASM_ARCH_CLOCK_H
#define _ASM_ARCH_CLOCK_H

struct udevice;

/* define pll mode */
#define RKCLK_PLL_MODE_SLOW		0
#define RKCLK_PLL_MODE_NORMAL		1
#define RKCLK_PLL_MODE_DEEP		2

enum {
	ROCKCHIP_SYSCON_NOC,
	ROCKCHIP_SYSCON_GRF,
	ROCKCHIP_SYSCON_SGRF,
	ROCKCHIP_SYSCON_PMU,
	ROCKCHIP_SYSCON_PMUGRF,
	ROCKCHIP_SYSCON_PMUSGRF,
	ROCKCHIP_SYSCON_CIC,
	ROCKCHIP_SYSCON_MSCH,
	ROCKCHIP_SYSCON_USBGRF,
	ROCKCHIP_SYSCON_PCIE30_PHY_GRF,
	ROCKCHIP_SYSCON_PHP_GRF,
	ROCKCHIP_SYSCON_PIPE_PHY0_GRF,
	ROCKCHIP_SYSCON_PIPE_PHY1_GRF,
	ROCKCHIP_SYSCON_PIPE_PHY2_GRF,
	ROCKCHIP_SYSCON_VOP_GRF,
	ROCKCHIP_SYSCON_VO_GRF,
};

/* Standard Rockchip clock numbers */
enum rk_clk_id {
	CLK_OSC,
	CLK_ARM,
	CLK_DDR,
	CLK_CODEC,
	CLK_GENERAL,
	CLK_NEW,

	CLK_COUNT,
};

#define PLL(_type, _id, _con, _mode, _mshift,			\
		 _lshift, _pflags, _rtable)			\
	{							\
		.id		= _id,				\
		.type		= _type,			\
		.con_offset	= _con,				\
		.mode_offset	= _mode,			\
		.mode_shift	= _mshift,			\
		.lock_shift	= _lshift,			\
		.pll_flags	= _pflags,			\
		.rate_table	= _rtable,			\
	}

#define RK3036_PLL_RATE(_rate, _refdiv, _fbdiv, _postdiv1,	\
			_postdiv2, _dsmpd, _frac)		\
{								\
	.rate	= _rate##U,					\
	.fbdiv = _fbdiv,					\
	.postdiv1 = _postdiv1,					\
	.refdiv = _refdiv,					\
	.postdiv2 = _postdiv2,					\
	.dsmpd = _dsmpd,					\
	.frac = _frac,						\
}

#define RK3588_PLL_RATE(_rate, _p, _m, _s, _k)			\
{								\
	.rate	= _rate##U,					\
	.p = _p,						\
	.m = _m,						\
	.s = _s,						\
	.k = _k,						\
}

struct rockchip_pll_rate_table {
	unsigned long rate;
	unsigned int nr;
	unsigned int nf;
	unsigned int no;
	unsigned int nb;
	/* for RK3036/RK3399 */
	unsigned int fbdiv;
	unsigned int postdiv1;
	unsigned int refdiv;
	unsigned int postdiv2;
	unsigned int dsmpd;
	unsigned int frac;
	/* for RK3588 */
	unsigned int m;
	unsigned int p;
	unsigned int s;
	unsigned int k;
};

enum rockchip_pll_type {
	pll_rk3036,
	pll_rk3066,
	pll_rk3328,
	pll_rk3366,
	pll_rk3399,
	pll_rk3588,
};

struct rockchip_pll_clock {
	unsigned int			id;
	unsigned int			con_offset;
	unsigned int			mode_offset;
	unsigned int			mode_shift;
	unsigned int			lock_shift;
	enum rockchip_pll_type		type;
	unsigned int			pll_flags;
	struct rockchip_pll_rate_table *rate_table;
	unsigned int			mode_mask;
};

struct rockchip_cpu_rate_table {
	unsigned long rate;
	unsigned int aclk_div;
	unsigned int pclk_div;
};

int rockchip_pll_set_rate(struct rockchip_pll_clock *pll,
			  void __iomem *base, ulong clk_id,
			  ulong drate);
ulong rockchip_pll_get_rate(struct rockchip_pll_clock *pll,
			    void __iomem *base, ulong clk_id);
const struct rockchip_cpu_rate_table *
rockchip_get_cpu_settings(struct rockchip_cpu_rate_table *cpu_table,
			  ulong rate);

static inline int rk_pll_id(enum rk_clk_id clk_id)
{
	return clk_id - 1;
}

struct sysreset_reg {
	unsigned int glb_srst_fst_value;
	unsigned int glb_srst_snd_value;
};

/**
 * clk_get_divisor() - Calculate the required clock divisior
 *
 * Given an input rate and a required output_rate, calculate the Rockchip
 * divisor needed to achieve this.
 *
 * @input_rate:		Input clock rate in Hz
 * @output_rate:	Output clock rate in Hz
 * Return: divisor register value to use
 */
static inline u32 clk_get_divisor(ulong input_rate, uint output_rate)
{
	uint clk_div;

	clk_div = input_rate / output_rate;
	clk_div = (clk_div + 1) & 0xfffe;

	return clk_div;
}

/**
 * rockchip_get_cru() - get a pointer to the clock/reset unit registers
 *
 * Return: pointer to registers, or -ve error on error
 */
void *rockchip_get_cru(void);

/**
 * rockchip_get_pmucru() - get a pointer to the clock/reset unit registers
 *
 * Return: pointer to registers, or -ve error on error
 */
void *rockchip_get_pmucru(void);

struct rockchip_cru;
struct rk3288_grf;

void rk3288_clk_configure_cpu(struct rockchip_cru *cru, struct rk3288_grf *grf);

int rockchip_get_clk(struct udevice **devp);

/*
 * rockchip_reset_bind() - Bind soft reset device as child of clock device
 *
 * @pdev: clock udevice
 * @reg_offset: the first offset in cru for softreset registers
 * @reg_number: the reg numbers of softreset registers
 * Return: 0 success, or error value
 */
int rockchip_reset_bind(struct udevice *pdev, u32 reg_offset, u32 reg_number);
/*
 * rockchip_reset_bind_lut() - Bind soft reset device as child of clock device
 *			       using a dedicated SoC lookup table
 * @pdev: clock udevice
 * @lookup_table: register lookup_table dedicated to SoC
 * @reg_offset: the first offset in cru for softreset registers
 * @reg_number: the reg numbers of softreset registers
 * Return: 0 success, or error value
 */
int rockchip_reset_bind_lut(struct udevice *pdev, const int *lookup_table,
			    u32 reg_offset, u32 reg_number);
/*
 * rk3588_reset_bind_lut() - Bind soft reset device as child of clock device
 *			     using dedicated RK3588 lookup table
 *
 * @pdev: clock udevice
 * @reg_offset: the first offset in cru for softreset registers
 * @reg_number: the reg numbers of softreset registers
 * Return: 0 success, or error value
 */
int rk3588_reset_bind_lut(struct udevice *pdev, u32 reg_offset, u32 reg_number);

#endif
