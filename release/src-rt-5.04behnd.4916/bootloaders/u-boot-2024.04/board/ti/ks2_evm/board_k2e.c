// SPDX-License-Identifier: GPL-2.0+
/*
 * K2E EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <image.h>
#include <init.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/keystone_net.h>

unsigned int get_external_clk(u32 clk)
{
	unsigned int clk_freq;

	switch (clk) {
	case sys_clk:
		clk_freq = 100000000;
		break;
	case alt_core_clk:
		clk_freq = 100000000;
		break;
	case pa_clk:
		clk_freq = 100000000;
		break;
	case ddr3a_clk:
		clk_freq = 100000000;
		break;
	default:
		clk_freq = 0;
		break;
	}

	return clk_freq;
}

static struct pll_init_data core_pll_config[NUM_SPDS] = {
	[SPD800]	= CORE_PLL_800,
	[SPD850]	= CORE_PLL_850,
	[SPD1000]	= CORE_PLL_1000,
	[SPD1250]	= CORE_PLL_1250,
	[SPD1350]	= CORE_PLL_1350,
	[SPD1400]	= CORE_PLL_1400,
	[SPD1500]	= CORE_PLL_1500,
};

/* DEV and ARM speed definitions as specified in DEVSPEED register */
int speeds[DEVSPEED_NUMSPDS] = {
	SPD850,
	SPD1000,
	SPD1250,
	SPD1350,
	SPD1400,
	SPD1500,
	SPD1400,
	SPD1350,
	SPD1250,
	SPD1000,
	SPD850,
	SPD800,
};

s16 divn_val[16] = {
	0, 0, 1, 4, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static struct pll_init_data pa_pll_config =
	PASS_PLL_1000;

struct pll_init_data *get_pll_init_data(int pll)
{
	int speed;
	struct pll_init_data *data;

	switch (pll) {
	case MAIN_PLL:
		speed = get_max_dev_speed(speeds);
		data = &core_pll_config[speed];
		break;
	case PASS_PLL:
		data = &pa_pll_config;
		break;
	default:
		data = NULL;
	}

	return data;
}

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (!strcmp(name, "keystone-k2e-evm"))
		return 0;

	return -1;
}
#endif

#if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	init_plls();

	return 0;
}
#endif

#ifdef CONFIG_SPL_BUILD
void spl_init_keystone_plls(void)
{
	init_plls();
}
#endif
