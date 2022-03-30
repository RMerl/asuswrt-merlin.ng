// SPDX-License-Identifier: GPL-2.0+
/*
 * K2L EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <common.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>
#include <asm/ti-common/keystone_net.h>

unsigned int get_external_clk(u32 clk)
{
	unsigned int clk_freq;

	switch (clk) {
	case sys_clk:
		clk_freq = 122880000;
		break;
	case alt_core_clk:
		clk_freq = 100000000;
		break;
	case pa_clk:
		clk_freq = 122880000;
		break;
	case tetris_clk:
		clk_freq = 122880000;
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
	[SPD800]	= CORE_PLL_799,
	[SPD1000]	= CORE_PLL_1000,
	[SPD1200]	= CORE_PLL_1198,
};

s16 divn_val[16] = {
	0, 0, 1, 4, 23, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static struct pll_init_data tetris_pll_config[] = {
	[SPD800]	= TETRIS_PLL_799,
	[SPD1000]	= TETRIS_PLL_1000,
	[SPD1200]	= TETRIS_PLL_1198,
	[SPD1350]	= TETRIS_PLL_1352,
	[SPD1400]	= TETRIS_PLL_1401,
};

static struct pll_init_data pa_pll_config =
	PASS_PLL_983;

struct pll_init_data *get_pll_init_data(int pll)
{
	int speed;
	struct pll_init_data *data;

	switch (pll) {
	case MAIN_PLL:
		speed = get_max_dev_speed(speeds);
		data = &core_pll_config[speed];
		break;
	case TETRIS_PLL:
		speed = get_max_arm_speed(speeds);
		data = &tetris_pll_config[speed];
		break;
	case PASS_PLL:
		data = &pa_pll_config;
		break;
	default:
		data = NULL;
	}

	return data;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	init_plls();

	return 0;
}
#endif

#if defined(CONFIG_MULTI_DTB_FIT)
int board_fit_config_name_match(const char *name)
{
	if (!strcmp(name, "keystone-k2l-evm"))
		return 0;

	return -1;
}
#endif

#ifdef CONFIG_SPL_BUILD
void spl_init_keystone_plls(void)
{
	init_plls();
}
#endif
