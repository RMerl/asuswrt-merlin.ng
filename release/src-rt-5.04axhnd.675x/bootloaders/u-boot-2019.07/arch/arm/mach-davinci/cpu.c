// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004 Texas Instruments.
 * Copyright (C) 2009 David Brownell
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* offsets from PLL controller base */
#define PLLC_PLLCTL	0x100
#define PLLC_PLLM	0x110
#define PLLC_PREDIV	0x114
#define PLLC_PLLDIV1	0x118
#define PLLC_PLLDIV2	0x11c
#define PLLC_PLLDIV3	0x120
#define PLLC_POSTDIV	0x128
#define PLLC_BPDIV	0x12c
#define PLLC_PLLDIV4	0x160
#define PLLC_PLLDIV5	0x164
#define PLLC_PLLDIV6	0x168
#define PLLC_PLLDIV7	0x16c
#define PLLC_PLLDIV8	0x170
#define PLLC_PLLDIV9	0x174

unsigned int sysdiv[9] = {
	PLLC_PLLDIV1, PLLC_PLLDIV2, PLLC_PLLDIV3, PLLC_PLLDIV4, PLLC_PLLDIV5,
	PLLC_PLLDIV6, PLLC_PLLDIV7, PLLC_PLLDIV8, PLLC_PLLDIV9
};

int clk_get(enum davinci_clk_ids id)
{
	int pre_div;
	int pllm;
	int post_div;
	int pll_out;
	unsigned int pll_base;

	pll_out = CONFIG_SYS_OSCIN_FREQ;

	if (id == DAVINCI_AUXCLK_CLKID)
		goto out;

	if ((id >> 16) == 1)
		pll_base = (unsigned int)davinci_pllc1_regs;
	else
		pll_base = (unsigned int)davinci_pllc0_regs;

	id &= 0xFFFF;

	/*
	 * Lets keep this simple. Combining operations can result in
	 * unexpected approximations
	 */
	pre_div = (readl(pll_base + PLLC_PREDIV) &
		DAVINCI_PLLC_DIV_MASK) + 1;
	pllm = readl(pll_base + PLLC_PLLM) + 1;

	pll_out /= pre_div;
	pll_out *= pllm;

	if (id == DAVINCI_PLLM_CLKID)
		goto out;

	post_div = (readl(pll_base + PLLC_POSTDIV) &
		DAVINCI_PLLC_DIV_MASK) + 1;

	pll_out /= post_div;

	if (id == DAVINCI_PLLC_CLKID)
		goto out;

	pll_out /= (readl(pll_base + sysdiv[id - 1]) &
		DAVINCI_PLLC_DIV_MASK) + 1;

out:
	return pll_out;
}

int set_cpu_clk_info(void)
{
	gd->bd->bi_arm_freq = clk_get(DAVINCI_ARM_CLKID) / 1000000;
	/* DDR PHY uses an x2 input clock */
	gd->bd->bi_ddr_freq = cpu_is_da830() ? 0 :
				(clk_get(DAVINCI_DDR_CLKID) / 1000000);
	gd->bd->bi_dsp_freq = 0;
	return 0;
}

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int cpu_eth_init(bd_t *bis)
{
#if defined(CONFIG_DRIVER_TI_EMAC)
	davinci_emac_initialize();
#endif
	return 0;
}
