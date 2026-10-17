// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <asm/fsp/fsp_support.h>
#include <asm/global_data.h>

int dram_init(void)
{
	int ret;

	/* The FSP has already set up DRAM, so grab the info we need */
	ret = fsp_scan_for_ram_size();
	if (ret)
		return ret;

	if (IS_ENABLED(CONFIG_ENABLE_MRC_CACHE)) {
		struct mrc_output *mrc = &gd->arch.mrc[MRC_TYPE_NORMAL];

		mrc->buf = fsp_get_nvs_data(gd->arch.hob_list, &mrc->len);
	}

	return 0;
}

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	return fsp_get_usable_lowmem_top(gd->arch.hob_list);
}
