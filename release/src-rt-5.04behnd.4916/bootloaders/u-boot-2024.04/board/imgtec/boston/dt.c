// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include <common.h>
#include <fdt_support.h>
#include <asm/global_data.h>

int ft_board_setup(void *blob, struct bd_info *bd)
{
	DECLARE_GLOBAL_DATA_PTR;
	u64 mem_start[2], mem_size[2];
	int mem_regions;

	mem_start[0] = 0;
	mem_size[0] = min_t(u64, 256llu << 20, gd->ram_size);
	mem_regions = 1;

	if (gd->ram_size > mem_size[0]) {
		mem_start[1] = 0x80000000 + mem_size[0];
		mem_size[1] = gd->ram_size - mem_size[0];
		mem_regions++;
	}

	return fdt_fixup_memory_banks(blob, mem_start, mem_size, mem_regions);
}
