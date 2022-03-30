// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/clk.h>

#define CLK_M	0
#define CLK_D	1
#define CLK_P	2

#ifndef CONFIG_SYS_CLK_FREQ_C100
#define CONFIG_SYS_CLK_FREQ_C100	12000000
#endif
#ifndef CONFIG_SYS_CLK_FREQ_C110
#define CONFIG_SYS_CLK_FREQ_C110	24000000
#endif

/* s5pc110: return pll clock frequency */
static unsigned long s5pc100_get_pll_clk(int pllreg)
{
	struct s5pc100_clock *clk =
		(struct s5pc100_clock *)samsung_get_base_clock();
	unsigned long r, m, p, s, mask, fout;
	unsigned int freq;

	switch (pllreg) {
	case APLL:
		r = readl(&clk->apll_con);
		break;
	case MPLL:
		r = readl(&clk->mpll_con);
		break;
	case EPLL:
		r = readl(&clk->epll_con);
		break;
	case HPLL:
		r = readl(&clk->hpll_con);
		break;
	default:
		printf("Unsupported PLL (%d)\n", pllreg);
		return 0;
	}

	/*
	 * APLL_CON: MIDV [25:16]
	 * MPLL_CON: MIDV [23:16]
	 * EPLL_CON: MIDV [23:16]
	 * HPLL_CON: MIDV [23:16]
	 */
	if (pllreg == APLL)
		mask = 0x3ff;
	else
		mask = 0x0ff;

	m = (r >> 16) & mask;

	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	/* FOUT = MDIV * FIN / (PDIV * 2^SDIV) */
	freq = CONFIG_SYS_CLK_FREQ_C100;
	fout = m * (freq / (p * (1 << s)));

	return fout;
}

/* s5pc100: return pll clock frequency */
static unsigned long s5pc110_get_pll_clk(int pllreg)
{
	struct s5pc110_clock *clk =
		(struct s5pc110_clock *)samsung_get_base_clock();
	unsigned long r, m, p, s, mask, fout;
	unsigned int freq;

	switch (pllreg) {
	case APLL:
		r = readl(&clk->apll_con);
		break;
	case MPLL:
		r = readl(&clk->mpll_con);
		break;
	case EPLL:
		r = readl(&clk->epll_con);
		break;
	case VPLL:
		r = readl(&clk->vpll_con);
		break;
	default:
		printf("Unsupported PLL (%d)\n", pllreg);
		return 0;
	}

	/*
	 * APLL_CON: MIDV [25:16]
	 * MPLL_CON: MIDV [25:16]
	 * EPLL_CON: MIDV [24:16]
	 * VPLL_CON: MIDV [24:16]
	 */
	if (pllreg == APLL || pllreg == MPLL)
		mask = 0x3ff;
	else
		mask = 0x1ff;

	m = (r >> 16) & mask;

	/* PDIV [13:8] */
	p = (r >> 8) & 0x3f;
	/* SDIV [2:0] */
	s = r & 0x7;

	freq = CONFIG_SYS_CLK_FREQ_C110;
	if (pllreg == APLL) {
		if (s < 1)
			s = 1;
		/* FOUT = MDIV * FIN / (PDIV * 2^(SDIV - 1)) */
		fout = m * (freq / (p * (1 << (s - 1))));
	} else
		/* FOUT = MDIV * FIN / (PDIV * 2^SDIV) */
		fout = m * (freq / (p * (1 << s)));

	return fout;
}

/* s5pc110: return ARM clock frequency */
static unsigned long s5pc110_get_arm_clk(void)
{
	struct s5pc110_clock *clk =
		(struct s5pc110_clock *)samsung_get_base_clock();
	unsigned long div;
	unsigned long dout_apll, armclk;
	unsigned int apll_ratio;

	div = readl(&clk->div0);

	/* APLL_RATIO: [2:0] */
	apll_ratio = div & 0x7;

	dout_apll = get_pll_clk(APLL) / (apll_ratio + 1);
	armclk = dout_apll;

	return armclk;
}

/* s5pc100: return ARM clock frequency */
static unsigned long s5pc100_get_arm_clk(void)
{
	struct s5pc100_clock *clk =
		(struct s5pc100_clock *)samsung_get_base_clock();
	unsigned long div;
	unsigned long dout_apll, armclk;
	unsigned int apll_ratio, arm_ratio;

	div = readl(&clk->div0);

	/* ARM_RATIO: [6:4] */
	arm_ratio = (div >> 4) & 0x7;
	/* APLL_RATIO: [0] */
	apll_ratio = div & 0x1;

	dout_apll = get_pll_clk(APLL) / (apll_ratio + 1);
	armclk = dout_apll / (arm_ratio + 1);

	return armclk;
}

/* s5pc100: return HCLKD0 frequency */
static unsigned long get_hclk(void)
{
	struct s5pc100_clock *clk =
		(struct s5pc100_clock *)samsung_get_base_clock();
	unsigned long hclkd0;
	uint div, d0_bus_ratio;

	div = readl(&clk->div0);
	/* D0_BUS_RATIO: [10:8] */
	d0_bus_ratio = (div >> 8) & 0x7;

	hclkd0 = get_arm_clk() / (d0_bus_ratio + 1);

	return hclkd0;
}

/* s5pc100: return PCLKD1 frequency */
static unsigned long get_pclkd1(void)
{
	struct s5pc100_clock *clk =
		(struct s5pc100_clock *)samsung_get_base_clock();
	unsigned long d1_bus, pclkd1;
	uint div, d1_bus_ratio, pclkd1_ratio;

	div = readl(&clk->div0);
	/* D1_BUS_RATIO: [14:12] */
	d1_bus_ratio = (div >> 12) & 0x7;
	/* PCLKD1_RATIO: [18:16] */
	pclkd1_ratio = (div >> 16) & 0x7;

	/* ASYNC Mode */
	d1_bus = get_pll_clk(MPLL) / (d1_bus_ratio + 1);
	pclkd1 = d1_bus / (pclkd1_ratio + 1);

	return pclkd1;
}

/* s5pc110: return HCLKs frequency */
static unsigned long get_hclk_sys(int dom)
{
	struct s5pc110_clock *clk =
		(struct s5pc110_clock *)samsung_get_base_clock();
	unsigned long hclk;
	unsigned int div;
	unsigned int offset;
	unsigned int hclk_sys_ratio;

	if (dom == CLK_M)
		return get_hclk();

	div = readl(&clk->div0);

	/*
	 * HCLK_MSYS_RATIO: [10:8]
	 * HCLK_DSYS_RATIO: [19:16]
	 * HCLK_PSYS_RATIO: [27:24]
	 */
	offset = 8 + (dom << 0x3);

	hclk_sys_ratio = (div >> offset) & 0xf;

	hclk = get_pll_clk(MPLL) / (hclk_sys_ratio + 1);

	return hclk;
}

/* s5pc110: return PCLKs frequency */
static unsigned long get_pclk_sys(int dom)
{
	struct s5pc110_clock *clk =
		(struct s5pc110_clock *)samsung_get_base_clock();
	unsigned long pclk;
	unsigned int div;
	unsigned int offset;
	unsigned int pclk_sys_ratio;

	div = readl(&clk->div0);

	/*
	 * PCLK_MSYS_RATIO: [14:12]
	 * PCLK_DSYS_RATIO: [22:20]
	 * PCLK_PSYS_RATIO: [30:28]
	 */
	offset = 12 + (dom << 0x3);

	pclk_sys_ratio = (div >> offset) & 0x7;

	pclk = get_hclk_sys(dom) / (pclk_sys_ratio + 1);

	return pclk;
}

/* s5pc110: return peripheral clock frequency */
static unsigned long s5pc110_get_pclk(void)
{
	return get_pclk_sys(CLK_P);
}

/* s5pc100: return peripheral clock frequency */
static unsigned long s5pc100_get_pclk(void)
{
	return get_pclkd1();
}

/* s5pc1xx: return uart clock frequency */
static unsigned long s5pc1xx_get_uart_clk(int dev_index)
{
	if (cpu_is_s5pc110())
		return s5pc110_get_pclk();
	else
		return s5pc100_get_pclk();
}

/* s5pc1xx: return pwm clock frequency */
static unsigned long s5pc1xx_get_pwm_clk(void)
{
	if (cpu_is_s5pc110())
		return s5pc110_get_pclk();
	else
		return s5pc100_get_pclk();
}

unsigned long get_pll_clk(int pllreg)
{
	if (cpu_is_s5pc110())
		return s5pc110_get_pll_clk(pllreg);
	else
		return s5pc100_get_pll_clk(pllreg);
}

unsigned long get_arm_clk(void)
{
	if (cpu_is_s5pc110())
		return s5pc110_get_arm_clk();
	else
		return s5pc100_get_arm_clk();
}

unsigned long get_pwm_clk(void)
{
	return s5pc1xx_get_pwm_clk();
}

unsigned long get_uart_clk(int dev_index)
{
	return s5pc1xx_get_uart_clk(dev_index);
}

void set_mmc_clk(int dev_index, unsigned int div)
{
	/* Do NOTHING */
}
