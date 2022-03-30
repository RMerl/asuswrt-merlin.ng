/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015 Google, Inc
 */

#ifndef _ASM_ARCH_CLOCK_H
#define _ASM_ARCH_CLOCK_H

/* define pll mode */
#define RKCLK_PLL_MODE_SLOW		0
#define RKCLK_PLL_MODE_NORMAL		1

enum {
	ROCKCHIP_SYSCON_NOC,
	ROCKCHIP_SYSCON_GRF,
	ROCKCHIP_SYSCON_SGRF,
	ROCKCHIP_SYSCON_PMU,
	ROCKCHIP_SYSCON_PMUGRF,
	ROCKCHIP_SYSCON_PMUSGRF,
	ROCKCHIP_SYSCON_CIC,
	ROCKCHIP_SYSCON_MSCH,
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

static inline int rk_pll_id(enum rk_clk_id clk_id)
{
	return clk_id - 1;
}

struct sysreset_reg {
	unsigned int glb_srst_fst_value;
	unsigned int glb_srst_snd_value;
};

struct softreset_reg {
        void __iomem *base;
        unsigned int sf_reset_offset;
        unsigned int sf_reset_num;
};

/**
 * clk_get_divisor() - Calculate the required clock divisior
 *
 * Given an input rate and a required output_rate, calculate the Rockchip
 * divisor needed to achieve this.
 *
 * @input_rate:		Input clock rate in Hz
 * @output_rate:	Output clock rate in Hz
 * @return divisor register value to use
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
 * @return pointer to registers, or -ve error on error
 */
void *rockchip_get_cru(void);

/**
 * rockchip_get_pmucru() - get a pointer to the clock/reset unit registers
 *
 * @return pointer to registers, or -ve error on error
 */
void *rockchip_get_pmucru(void);

struct rk3288_cru;
struct rk3288_grf;

void rk3288_clk_configure_cpu(struct rk3288_cru *cru, struct rk3288_grf *grf);

int rockchip_get_clk(struct udevice **devp);

/*
 * rockchip_reset_bind() - Bind soft reset device as child of clock device
 *
 * @pdev: clock udevice
 * @reg_offset: the first offset in cru for softreset registers
 * @reg_number: the reg numbers of softreset registers
 * @return 0 success, or error value
 */
int rockchip_reset_bind(struct udevice *pdev, u32 reg_offset, u32 reg_number);

#endif
