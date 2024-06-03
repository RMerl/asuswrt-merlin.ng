// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/sizes.h>
#include <asm/global_data.h>

#include "sg-regs.h"
#include "soc-info.h"

DECLARE_GLOBAL_DATA_PTR;

struct uniphier_memif_data {
	unsigned int soc_id;
	unsigned long sparse_ch1_base;
	int have_ch2;
};

static const struct uniphier_memif_data uniphier_memif_data[] = {
	{
		.soc_id = UNIPHIER_LD4_ID,
		.sparse_ch1_base = 0xc0000000,
	},
	{
		.soc_id = UNIPHIER_PRO4_ID,
		.sparse_ch1_base = 0xa0000000,
	},
	{
		.soc_id = UNIPHIER_SLD8_ID,
		.sparse_ch1_base = 0xc0000000,
	},
	{
		.soc_id = UNIPHIER_PRO5_ID,
		.sparse_ch1_base = 0xc0000000,
	},
	{
		.soc_id = UNIPHIER_PXS2_ID,
		.sparse_ch1_base = 0xc0000000,
		.have_ch2 = 1,
	},
	{
		.soc_id = UNIPHIER_LD6B_ID,
		.sparse_ch1_base = 0xc0000000,
		.have_ch2 = 1,
	},
	{
		.soc_id = UNIPHIER_LD11_ID,
		.sparse_ch1_base = 0xc0000000,
	},
	{
		.soc_id = UNIPHIER_LD20_ID,
		.sparse_ch1_base = 0xc0000000,
		.have_ch2 = 1,
	},
	{
		.soc_id = UNIPHIER_PXS3_ID,
		.sparse_ch1_base = 0xc0000000,
		.have_ch2 = 1,
	},
};
UNIPHIER_DEFINE_SOCDATA_FUNC(uniphier_get_memif_data, uniphier_memif_data)

struct uniphier_dram_map {
	unsigned long base;
	unsigned long size;
};

static int uniphier_memconf_decode(struct uniphier_dram_map *dram_map)
{
	const struct uniphier_memif_data *data;
	unsigned long size;
	u32 val;

	data = uniphier_get_memif_data();
	if (!data) {
		pr_err("unsupported SoC\n");
		return -EINVAL;
	}

	val = readl(SG_MEMCONF);

	/* set up ch0 */
	dram_map[0].base = CONFIG_SYS_SDRAM_BASE;

	switch (val & SG_MEMCONF_CH0_SZ_MASK) {
	case SG_MEMCONF_CH0_SZ_64M:
		size = SZ_64M;
		break;
	case SG_MEMCONF_CH0_SZ_128M:
		size = SZ_128M;
		break;
	case SG_MEMCONF_CH0_SZ_256M:
		size = SZ_256M;
		break;
	case SG_MEMCONF_CH0_SZ_512M:
		size = SZ_512M;
		break;
	case SG_MEMCONF_CH0_SZ_1G:
		size = SZ_1G;
		break;
	default:
		pr_err("error: invalid value is set to MEMCONF ch0 size\n");
		return -EINVAL;
	}

	if ((val & SG_MEMCONF_CH0_NUM_MASK) == SG_MEMCONF_CH0_NUM_2)
		size *= 2;

	dram_map[0].size = size;

	/* set up ch1 */
	dram_map[1].base = dram_map[0].base + size;

	if (val & SG_MEMCONF_SPARSEMEM) {
		if (dram_map[1].base > data->sparse_ch1_base) {
			pr_warn("Sparse mem is enabled, but ch0 and ch1 overlap\n");
			pr_warn("Only ch0 is available\n");
			dram_map[1].base = 0;
			return 0;
		}

		dram_map[1].base = data->sparse_ch1_base;
	}

	switch (val & SG_MEMCONF_CH1_SZ_MASK) {
	case SG_MEMCONF_CH1_SZ_64M:
		size = SZ_64M;
		break;
	case SG_MEMCONF_CH1_SZ_128M:
		size = SZ_128M;
		break;
	case SG_MEMCONF_CH1_SZ_256M:
		size = SZ_256M;
		break;
	case SG_MEMCONF_CH1_SZ_512M:
		size = SZ_512M;
		break;
	case SG_MEMCONF_CH1_SZ_1G:
		size = SZ_1G;
		break;
	default:
		pr_err("error: invalid value is set to MEMCONF ch1 size\n");
		return -EINVAL;
	}

	if ((val & SG_MEMCONF_CH1_NUM_MASK) == SG_MEMCONF_CH1_NUM_2)
		size *= 2;

	dram_map[1].size = size;

	if (!data->have_ch2 || val & SG_MEMCONF_CH2_DISABLE)
		return 0;

	/* set up ch2 */
	dram_map[2].base = dram_map[1].base + size;

	switch (val & SG_MEMCONF_CH2_SZ_MASK) {
	case SG_MEMCONF_CH2_SZ_64M:
		size = SZ_64M;
		break;
	case SG_MEMCONF_CH2_SZ_128M:
		size = SZ_128M;
		break;
	case SG_MEMCONF_CH2_SZ_256M:
		size = SZ_256M;
		break;
	case SG_MEMCONF_CH2_SZ_512M:
		size = SZ_512M;
		break;
	case SG_MEMCONF_CH2_SZ_1G:
		size = SZ_1G;
		break;
	default:
		pr_err("error: invalid value is set to MEMCONF ch2 size\n");
		return -EINVAL;
	}

	if ((val & SG_MEMCONF_CH2_NUM_MASK) == SG_MEMCONF_CH2_NUM_2)
		size *= 2;

	dram_map[2].size = size;

	return 0;
}

int dram_init(void)
{
	struct uniphier_dram_map dram_map[3] = {};
	int ret, i;

	gd->ram_size = 0;

	ret = uniphier_memconf_decode(dram_map);
	if (ret)
		return ret;

	for (i = 0; i < ARRAY_SIZE(dram_map); i++) {
		unsigned long max_size;

		if (!dram_map[i].size)
			break;

		/*
		 * U-Boot relocates itself to the tail of the memory region,
		 * but it does not expect sparse memory.  We use the first
		 * contiguous chunk here.
		 */
		if (i > 0 && dram_map[i - 1].base + dram_map[i - 1].size <
							dram_map[i].base)
			break;

		/*
		 * Do not use memory that exceeds 32bit address range.  U-Boot
		 * relocates itself to the end of the effectively available RAM.
		 * This could be a problem for DMA engines that do not support
		 * 64bit address (SDMA of SDHCI, UniPhier AV-ether, etc.)
		 */
		if (dram_map[i].base >= 1ULL << 32)
			break;

		max_size = (1ULL << 32) - dram_map[i].base;

		if (dram_map[i].size > max_size) {
			gd->ram_size += max_size;
			break;
		}

		gd->ram_size += dram_map[i].size;
	}

	/*
	 * LD20 uses the last 64 byte for each channel for dynamic
	 * DDR PHY training
	 */
	if (uniphier_get_soc_id() == UNIPHIER_LD20_ID)
		gd->ram_size -= 64;

	return 0;
}

int dram_init_banksize(void)
{
	struct uniphier_dram_map dram_map[3] = {};
	int i;

	uniphier_memconf_decode(dram_map);

	for (i = 0; i < ARRAY_SIZE(dram_map); i++) {
		if (i >= ARRAY_SIZE(gd->bd->bi_dram))
			break;

		gd->bd->bi_dram[i].start = dram_map[i].base;
		gd->bd->bi_dram[i].size = dram_map[i].size;
	}

	return 0;
}
