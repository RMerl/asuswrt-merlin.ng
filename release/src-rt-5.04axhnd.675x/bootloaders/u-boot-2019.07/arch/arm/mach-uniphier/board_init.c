// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/errno.h>
#include <linux/io.h>
#include <linux/printk.h>

#include "init.h"
#include "micro-support-card.h"
#include "soc-info.h"

#ifdef CONFIG_ARCH_UNIPHIER_LD20
static void uniphier_ld20_misc_init(void)
{
	/* ES1 errata: increase VDD09 supply to suppress VBO noise */
	if (uniphier_get_soc_revision() == 1) {
		writel(0x00000003, 0x6184e004);
		writel(0x00000100, 0x6184e040);
		writel(0x0000b500, 0x6184e024);
		writel(0x00000001, 0x6184e000);
	}
}
#endif

struct uniphier_initdata {
	unsigned int soc_id;
	void (*sbc_init)(void);
	void (*pll_init)(void);
	void (*clk_init)(void);
	void (*misc_init)(void);
};

static const struct uniphier_initdata uniphier_initdata[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	{
		.soc_id = UNIPHIER_LD4_ID,
		.sbc_init = uniphier_ld4_sbc_init,
		.pll_init = uniphier_ld4_pll_init,
		.clk_init = uniphier_ld4_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	{
		.soc_id = UNIPHIER_PRO4_ID,
		.sbc_init = uniphier_sbc_init_savepin,
		.pll_init = uniphier_pro4_pll_init,
		.clk_init = uniphier_pro4_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	{
		.soc_id = UNIPHIER_SLD8_ID,
		.sbc_init = uniphier_ld4_sbc_init,
		.pll_init = uniphier_ld4_pll_init,
		.clk_init = uniphier_ld4_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	{
		.soc_id = UNIPHIER_PRO5_ID,
		.sbc_init = uniphier_sbc_init_savepin,
		.clk_init = uniphier_pro5_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	{
		.soc_id = UNIPHIER_PXS2_ID,
		.sbc_init = uniphier_pxs2_sbc_init,
		.clk_init = uniphier_pxs2_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	{
		.soc_id = UNIPHIER_LD6B_ID,
		.sbc_init = uniphier_pxs2_sbc_init,
		.clk_init = uniphier_pxs2_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD11)
	{
		.soc_id = UNIPHIER_LD11_ID,
		.sbc_init = uniphier_ld11_sbc_init,
		.pll_init = uniphier_ld11_pll_init,
		.clk_init = uniphier_ld11_clk_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	{
		.soc_id = UNIPHIER_LD20_ID,
		.sbc_init = uniphier_ld11_sbc_init,
		.pll_init = uniphier_ld20_pll_init,
		.clk_init = uniphier_ld20_clk_init,
		.misc_init = uniphier_ld20_misc_init,
	},
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS3)
	{
		.soc_id = UNIPHIER_PXS3_ID,
		.sbc_init = uniphier_pxs2_sbc_init,
		.pll_init = uniphier_pxs3_pll_init,
		.clk_init = uniphier_pxs3_clk_init,
	},
#endif
};
UNIPHIER_DEFINE_SOCDATA_FUNC(uniphier_get_initdata, uniphier_initdata)

int board_init(void)
{
	const struct uniphier_initdata *initdata;

	led_puts("U0");

	initdata = uniphier_get_initdata();
	if (!initdata) {
		pr_err("unsupported SoC\n");
		return -EINVAL;
	}

	initdata->sbc_init();

	support_card_init();

	led_puts("U0");

	if (initdata->pll_init)
		initdata->pll_init();

	led_puts("U1");

	if (initdata->clk_init)
		initdata->clk_init();

	led_puts("U2");

	if (initdata->misc_init)
		initdata->misc_init();

	led_puts("U3");

	support_card_late_init();

	led_puts("Uboo");

	return 0;
}
