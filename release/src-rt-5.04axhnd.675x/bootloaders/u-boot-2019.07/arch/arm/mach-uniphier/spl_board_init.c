// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include "init.h"
#include "micro-support-card.h"
#include "soc-info.h"

struct uniphier_spl_initdata {
	unsigned int soc_id;
	void (*bcu_init)(const struct uniphier_board_data *bd);
	void (*early_clk_init)(void);
	int (*dpll_init)(const struct uniphier_board_data *bd);
	int (*memconf_init)(const struct uniphier_board_data *bd);
	void (*dram_clk_init)(void);
	int (*umc_init)(const struct uniphier_board_data *bd);
};

static const struct uniphier_spl_initdata uniphier_spl_initdata[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	{
		.soc_id = UNIPHIER_LD4_ID,
		.bcu_init = uniphier_ld4_bcu_init,
		.early_clk_init = uniphier_ld4_early_clk_init,
		.dpll_init = uniphier_ld4_dpll_init,
		.memconf_init = uniphier_memconf_2ch_init,
		.dram_clk_init = uniphier_ld4_dram_clk_init,
		.umc_init = uniphier_ld4_umc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	{
		.soc_id = UNIPHIER_PRO4_ID,
		.early_clk_init = uniphier_ld4_early_clk_init,
		.dpll_init = uniphier_pro4_dpll_init,
		.memconf_init = uniphier_memconf_2ch_init,
		.dram_clk_init = uniphier_ld4_dram_clk_init,
		.umc_init = uniphier_pro4_umc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	{
		.soc_id = UNIPHIER_SLD8_ID,
		.bcu_init = uniphier_ld4_bcu_init,
		.early_clk_init = uniphier_ld4_early_clk_init,
		.dpll_init = uniphier_sld8_dpll_init,
		.memconf_init = uniphier_memconf_2ch_init,
		.dram_clk_init = uniphier_ld4_dram_clk_init,
		.umc_init = uniphier_sld8_umc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	{
		.soc_id = UNIPHIER_PRO5_ID,
		.early_clk_init = uniphier_ld4_early_clk_init,
		.dpll_init = uniphier_pro5_dpll_init,
		.memconf_init = uniphier_memconf_2ch_init,
		.dram_clk_init = uniphier_pro5_dram_clk_init,
		.umc_init = uniphier_pro5_umc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	{
		.soc_id = UNIPHIER_PXS2_ID,
		.early_clk_init = uniphier_ld4_early_clk_init,
		.dpll_init = uniphier_pxs2_dpll_init,
		.memconf_init = uniphier_memconf_3ch_init,
		.dram_clk_init = uniphier_pxs2_dram_clk_init,
		.umc_init = uniphier_pxs2_umc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	{
		.soc_id = UNIPHIER_LD6B_ID,
		.early_clk_init = uniphier_ld4_early_clk_init,
		.dpll_init = uniphier_pxs2_dpll_init,
		.memconf_init = uniphier_memconf_3ch_init,
		.dram_clk_init = uniphier_pxs2_dram_clk_init,
		.umc_init = uniphier_pxs2_umc_init,
	},
#endif
};
UNIPHIER_DEFINE_SOCDATA_FUNC(uniphier_get_spl_initdata, uniphier_spl_initdata)

void spl_board_init(void)
{
	const struct uniphier_board_data *bd;
	const struct uniphier_spl_initdata *initdata;
	int ret;

#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	bd = uniphier_get_board_param();
	if (!bd)
		hang();

	initdata = uniphier_get_spl_initdata();
	if (!initdata)
		hang();

	if (initdata->bcu_init)
		initdata->bcu_init(bd);

	initdata->early_clk_init();

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

	ret = initdata->dpll_init(bd);
	if (ret) {
		pr_err("failed to init DPLL\n");
		hang();
	}

	ret = initdata->memconf_init(bd);
	if (ret) {
		pr_err("failed to init MEMCONF\n");
		hang();
	}

	initdata->dram_clk_init();

	ret = initdata->umc_init(bd);
	if (ret) {
		pr_err("failed to init DRAM\n");
		hang();
	}
}
