// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/libfdt.h>
#include <linux/kernel.h>
#include <asm/global_data.h>

#include "init.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_ARCH_UNIPHIER_LD4)
static const struct uniphier_board_data uniphier_ld4_data = {
	.dram_freq = 1600,
	.dram_ch[0] = {
		.size = 0x10000000,
		.width = 16,
	},
	.dram_ch[1] = {
		.size = 0x10000000,
		.width = 16,
	},
	.flags = UNIPHIER_BD_DDR3PLUS,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
/* 1GB RAM board */
static const struct uniphier_board_data uniphier_pro4_data = {
	.dram_freq = 1600,
	.dram_ch[0] = {
		.size = 0x20000000,
		.width = 32,
	},
	.dram_ch[1] = {
		.size = 0x20000000,
		.width = 32,
	},
};

/* 2GB RAM board */
static const struct uniphier_board_data uniphier_pro4_2g_data = {
	.dram_freq = 1600,
	.dram_ch[0] = {
		.size = 0x40000000,
		.width = 32,
	},
	.dram_ch[1] = {
		.size = 0x40000000,
		.width = 32,
	},
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
static const struct uniphier_board_data uniphier_sld8_data = {
	.dram_freq = 1333,
	.dram_ch[0] = {
		.size = 0x10000000,
		.width = 16,
	},
	.dram_ch[1] = {
		.size = 0x10000000,
		.width = 16,
	},
	.flags = UNIPHIER_BD_DDR3PLUS,
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
static const struct uniphier_board_data uniphier_pro5_data = {
	.dram_freq = 1866,
	.dram_ch[0] = {
		.size = 0x20000000,
		.width = 32,
	},
	.dram_ch[1] = {
		.size = 0x20000000,
		.width = 32,
	},
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
static const struct uniphier_board_data uniphier_pxs2_data = {
	.dram_freq = 2133,
	.dram_ch[0] = {
		.size = 0x40000000,
		.width = 32,
	},
	.dram_ch[1] = {
		.size = 0x20000000,
		.width = 32,
	},
	.dram_ch[2] = {
		.size = 0x20000000,
		.width = 16,
	},
};
#endif

#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
static const struct uniphier_board_data uniphier_ld6b_data = {
	.dram_freq = 1866,
	.dram_ch[0] = {
		.size = 0x40000000,
		.width = 32,
	},
	.dram_ch[1] = {
		.size = 0x20000000,
		.width = 32,
	},
	.dram_ch[2] = {
		.size = 0x20000000,
		.width = 16,
	},
};
#endif

struct uniphier_board_id {
	const char *compatible;
	const struct uniphier_board_data *param;
};

static const struct uniphier_board_id uniphier_boards[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	{ "socionext,uniphier-ld4", &uniphier_ld4_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	{ "socionext,uniphier-pro4-ace", &uniphier_pro4_2g_data, },
	{ "socionext,uniphier-pro4-sanji", &uniphier_pro4_2g_data, },
	{ "socionext,uniphier-pro4", &uniphier_pro4_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	{ "socionext,uniphier-sld8", &uniphier_sld8_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	{ "socionext,uniphier-pro5", &uniphier_pro5_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	{ "socionext,uniphier-pxs2", &uniphier_pxs2_data, },
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	{ "socionext,uniphier-ld6b", &uniphier_ld6b_data, },
#endif
};

const struct uniphier_board_data *uniphier_get_board_param(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_boards); i++) {
		if (!fdt_node_check_compatible(gd->fdt_blob, 0,
					       uniphier_boards[i].compatible))
			return uniphier_boards[i].param;
	}

	return NULL;
}
