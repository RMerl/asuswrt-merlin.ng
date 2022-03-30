/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 */

#ifndef __ASM_ARM_ARCH_CLK_H_
#define __ASM_ARM_ARCH_CLK_H_

#define APLL	0
#define MPLL	1
#define EPLL	2
#define HPLL	3
#define VPLL	4
#define BPLL	5
#define RPLL	6
#define SPLL	7
#define CPLL	8
#define DPLL	9
#define IPLL	10

#define MASK_PRE_RATIO(x)	(0xff << ((x << 4) + 8))
#define MASK_RATIO(x)		(0xf << (x << 4))
#define SET_PRE_RATIO(x, y)	((y & 0xff) << ((x << 4) + 8))
#define SET_RATIO(x, y)		((y & 0xf) << (x << 4))

enum pll_src_bit {
	EXYNOS_SRC_MPLL = 6,
	EXYNOS_SRC_EPLL,
	EXYNOS_SRC_VPLL,
	EXYNOS542X_SRC_MPLL = 3,
	EXYNOS542X_SRC_SPLL,
	EXYNOS542X_SRC_EPLL = 6,
	EXYNOS542X_SRC_RPLL,
};

unsigned long get_pll_clk(int pllreg);
unsigned long get_arm_clk(void);
unsigned long get_i2c_clk(void);
unsigned long get_pwm_clk(void);
unsigned long get_uart_clk(int dev_index);
unsigned long get_mmc_clk(int dev_index);
void set_mmc_clk(int dev_index, unsigned int div);
unsigned long get_lcd_clk(void);
void set_lcd_clk(void);
void set_mipi_clk(void);
int set_i2s_clk_source(unsigned int i2s_id);
int set_i2s_clk_prescaler(unsigned int src_frq, unsigned int dst_frq,
				unsigned int i2s_id);
int set_epll_clk(unsigned long rate);
int set_spi_clk(int periph_id, unsigned int rate);

/**
 * get the clk frequency of the required peripheral
 *
 * @param peripheral	Peripheral id
 *
 * @return frequency of the peripheral clk
 */
unsigned long clock_get_periph_rate(int peripheral);

#endif
