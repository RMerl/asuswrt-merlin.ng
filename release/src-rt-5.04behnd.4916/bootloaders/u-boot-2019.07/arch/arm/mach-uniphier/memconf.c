// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2016      Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>

#include "sg-regs.h"
#include "init.h"

static int __uniphier_memconf_init(const struct uniphier_board_data *bd,
				   int have_ch2)
{
	u32 val = 0;
	unsigned long size_per_word;

	/* set up ch0 */
	switch (bd->dram_ch[0].width) {
	case 16:
		val |= SG_MEMCONF_CH0_NUM_1;
		size_per_word = bd->dram_ch[0].size;
		break;
	case 32:
		val |= SG_MEMCONF_CH0_NUM_2;
		size_per_word = bd->dram_ch[0].size >> 1;
		break;
	default:
		pr_err("error: unsupported DRAM ch0 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		val |= SG_MEMCONF_CH0_SZ_64M;
		break;
	case SZ_128M:
		val |= SG_MEMCONF_CH0_SZ_128M;
		break;
	case SZ_256M:
		val |= SG_MEMCONF_CH0_SZ_256M;
		break;
	case SZ_512M:
		val |= SG_MEMCONF_CH0_SZ_512M;
		break;
	case SZ_1G:
		val |= SG_MEMCONF_CH0_SZ_1G;
		break;
	default:
		pr_err("error: unsupported DRAM ch0 size\n");
		return -EINVAL;
	}

	/* set up ch1 */
	switch (bd->dram_ch[1].width) {
	case 16:
		val |= SG_MEMCONF_CH1_NUM_1;
		size_per_word = bd->dram_ch[1].size;
		break;
	case 32:
		val |= SG_MEMCONF_CH1_NUM_2;
		size_per_word = bd->dram_ch[1].size >> 1;
		break;
	default:
		pr_err("error: unsupported DRAM ch1 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		val |= SG_MEMCONF_CH1_SZ_64M;
		break;
	case SZ_128M:
		val |= SG_MEMCONF_CH1_SZ_128M;
		break;
	case SZ_256M:
		val |= SG_MEMCONF_CH1_SZ_256M;
		break;
	case SZ_512M:
		val |= SG_MEMCONF_CH1_SZ_512M;
		break;
	case SZ_1G:
		val |= SG_MEMCONF_CH1_SZ_1G;
		break;
	default:
		pr_err("error: unsupported DRAM ch1 size\n");
		return -EINVAL;
	}

	/* is sparse mem? */
	if (bd->flags & UNIPHIER_BD_DRAM_SPARSE)
		val |= SG_MEMCONF_SPARSEMEM;

	if (!have_ch2)
		goto out;

	if (!bd->dram_ch[2].size) {
		val |= SG_MEMCONF_CH2_DISABLE;
		goto out;
	}

	/* set up ch2 */
	switch (bd->dram_ch[2].width) {
	case 16:
		val |= SG_MEMCONF_CH2_NUM_1;
		size_per_word = bd->dram_ch[2].size;
		break;
	case 32:
		val |= SG_MEMCONF_CH2_NUM_2;
		size_per_word = bd->dram_ch[2].size >> 1;
		break;
	default:
		pr_err("error: unsupported DRAM ch2 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		val |= SG_MEMCONF_CH2_SZ_64M;
		break;
	case SZ_128M:
		val |= SG_MEMCONF_CH2_SZ_128M;
		break;
	case SZ_256M:
		val |= SG_MEMCONF_CH2_SZ_256M;
		break;
	case SZ_512M:
		val |= SG_MEMCONF_CH2_SZ_512M;
		break;
	case SZ_1G:
		val |= SG_MEMCONF_CH2_SZ_1G;
		break;
	default:
		pr_err("error: unsupported DRAM ch2 size\n");
		return -EINVAL;
	}

out:
	writel(val, SG_MEMCONF);

	return 0;
}

int uniphier_memconf_2ch_init(const struct uniphier_board_data *bd)
{
	return __uniphier_memconf_init(bd, 0);
}

int uniphier_memconf_3ch_init(const struct uniphier_board_data *bd)
{
	return __uniphier_memconf_init(bd, 1);
}
