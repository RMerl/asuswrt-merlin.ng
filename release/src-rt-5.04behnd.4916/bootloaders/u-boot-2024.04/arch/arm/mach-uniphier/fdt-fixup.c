// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016-2018 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <fdt_support.h>
#include <fdtdec.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/printk.h>

#include "soc-info.h"

/*
 * The DRAM PHY requires 64 byte scratch area in each DRAM channel
 * for its dynamic PHY training feature.
 */
static int uniphier_ld20_fdt_mem_rsv(void *fdt, struct bd_info *bd)
{
	unsigned long rsv_addr;
	const unsigned long rsv_size = 64;
	int i, ret;

	if (!IS_ENABLED(CONFIG_ARCH_UNIPHIER_LD20) ||
	    uniphier_get_soc_id() != UNIPHIER_LD20_ID)
		return 0;

	for (i = 0; i < ARRAY_SIZE(bd->bi_dram); i++) {
		if (!bd->bi_dram[i].size)
			continue;

		rsv_addr = bd->bi_dram[i].start + bd->bi_dram[i].size;
		rsv_addr -= rsv_size;

		ret = fdt_add_mem_rsv(fdt, rsv_addr, rsv_size);
		if (ret)
			return -ENOSPC;

		pr_notice("   Reserved memory region for DRAM PHY training: addr=%lx size=%lx\n",
			  rsv_addr, rsv_size);
	}

	return 0;
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	static const struct node_info nodes[] = {
		{ "socionext,uniphier-denali-nand-v5a", MTD_DEV_TYPE_NAND },
		{ "socionext,uniphier-denali-nand-v5b", MTD_DEV_TYPE_NAND },
	};
	int ret;

	fdt_fixup_mtdparts(fdt, nodes, ARRAY_SIZE(nodes));

	ret = uniphier_ld20_fdt_mem_rsv(fdt, bd);
	if (ret)
		return ret;

	return 0;
}
