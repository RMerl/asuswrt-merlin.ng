// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016 Imagination Technologies
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>

#include <asm/io.h>

#include "boston-regs.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	u32 ddrconf0 = __raw_readl((uint32_t *)BOSTON_PLAT_DDRCONF0);

	gd->ram_size = (phys_size_t)(ddrconf0 & BOSTON_PLAT_DDRCONF0_SIZE) <<
			30;

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->ram_top < CFG_SYS_SDRAM_BASE) {
		/* 2GB wrapped around to 0 */
		return CKSEG0ADDR(256 << 20);
	}

	return min_t(unsigned long, gd->ram_top, CKSEG0ADDR(256 << 20));
}
