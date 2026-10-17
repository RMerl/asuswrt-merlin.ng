// SPDX-License-Identifier: GPL-2.0+
/*
 * LS1028A TF-A calling support
 *
 * Copyright (c) 2020 Michael Walle <michael@walle.cc>
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <atf_common.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

struct region_info {
	u64 addr;
	u64 size;
};

struct dram_regions_info {
	u64 num_dram_regions;
	u64 total_dram_size;
	struct region_info region[CONFIG_NR_DRAM_BANKS];
};

struct bl_params *bl2_plat_get_bl31_params_v2(uintptr_t bl32_entry,
					      uintptr_t bl33_entry,
					      uintptr_t fdt_addr)
{
	static struct dram_regions_info dram_regions_info = { 0 };
	struct bl_params *bl_params;
	struct bl_params_node *node;
	void *dcfg_ccsr = (void *)DCFG_BASE;
	int i;

	dram_regions_info.num_dram_regions = CONFIG_NR_DRAM_BANKS;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		dram_regions_info.region[i].addr = gd->bd->bi_dram[i].start;
		dram_regions_info.region[i].size = gd->bd->bi_dram[i].size;
		dram_regions_info.total_dram_size += gd->bd->bi_dram[i].size;
	}

	bl_params = bl2_plat_get_bl31_params_v2_default(bl32_entry, bl33_entry,
							fdt_addr);

	for_each_bl_params_node(bl_params, node) {
		if (node->image_id == ATF_BL31_IMAGE_ID) {
			node->ep_info->args.arg3 = (uintptr_t)&dram_regions_info;
			node->ep_info->args.arg4 = in_le32(dcfg_ccsr + DCFG_PORSR1);
		}
	}

	return bl_params;
}
