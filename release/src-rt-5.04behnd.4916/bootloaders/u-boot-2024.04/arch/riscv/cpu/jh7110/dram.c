// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang <yanhong.wang@starfivetech.com>
 */

#include <fdtdec.h>
#include <init.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/*
	 * Ensure that we run from first 4GB so that all
	 * addresses used by U-Boot are 32bit addresses.
	 *
	 * This in-turn ensures that 32bit DMA capable
	 * devices work fine because DMA mapping APIs will
	 * provide 32bit DMA addresses only.
	 */
	if (IS_ENABLED(CONFIG_64BIT) && gd->ram_top > SZ_4G)
		return SZ_4G;

	return gd->ram_top;
}
