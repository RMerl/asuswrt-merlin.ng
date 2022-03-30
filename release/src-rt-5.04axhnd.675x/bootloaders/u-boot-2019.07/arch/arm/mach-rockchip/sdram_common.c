// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <asm/io.h>
#include <asm/arch-rockchip/sdram_common.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;
size_t rockchip_sdram_size(phys_addr_t reg)
{
	u32 rank, col, bk, cs0_row, cs1_row, bw, row_3_4;
	size_t chipsize_mb = 0;
	size_t size_mb = 0;
	u32 ch;

	u32 sys_reg = readl(reg);
	u32 ch_num = 1 + ((sys_reg >> SYS_REG_NUM_CH_SHIFT)
		       & SYS_REG_NUM_CH_MASK);

	debug("%s %x %x\n", __func__, (u32)reg, sys_reg);
	for (ch = 0; ch < ch_num; ch++) {
		rank = 1 + (sys_reg >> SYS_REG_RANK_SHIFT(ch) &
			SYS_REG_RANK_MASK);
		col = 9 + (sys_reg >> SYS_REG_COL_SHIFT(ch) & SYS_REG_COL_MASK);
		bk = 3 - ((sys_reg >> SYS_REG_BK_SHIFT(ch)) & SYS_REG_BK_MASK);
		cs0_row = 13 + (sys_reg >> SYS_REG_CS0_ROW_SHIFT(ch) &
				SYS_REG_CS0_ROW_MASK);
		cs1_row = 13 + (sys_reg >> SYS_REG_CS1_ROW_SHIFT(ch) &
				SYS_REG_CS1_ROW_MASK);
		bw = (2 >> ((sys_reg >> SYS_REG_BW_SHIFT(ch)) &
			SYS_REG_BW_MASK));
		row_3_4 = sys_reg >> SYS_REG_ROW_3_4_SHIFT(ch) &
			SYS_REG_ROW_3_4_MASK;

		chipsize_mb = (1 << (cs0_row + col + bk + bw - 20));

		if (rank > 1)
			chipsize_mb += chipsize_mb >> (cs0_row - cs1_row);
		if (row_3_4)
			chipsize_mb = chipsize_mb * 3 / 4;
		size_mb += chipsize_mb;
		debug("rank %d col %d bk %d cs0_row %d bw %d row_3_4 %d\n",
		      rank, col, bk, cs0_row, bw, row_3_4);
	}

	/*
	 * This is workaround for issue we can't get correct size for 4GB ram
	 * in 32bit system and available before we really need ram space
	 * out of 4GB, eg.enable ARM LAPE(rk3288 supports 8GB ram).
	 * The size of 4GB is '0x1 00000000', and this value will be truncated
	 * to 0 in 32bit system, and system can not get correct ram size.
	 * Rockchip SoCs reserve a blob of space for peripheral near 4GB,
	 * and we are now setting SDRAM_MAX_SIZE as max available space for
	 * ram in 4GB, so we can use this directly to workaround the issue.
	 * TODO:
	 *   1. update correct value for SDRAM_MAX_SIZE as what dram
	 *   controller sees.
	 *   2. update board_get_usable_ram_top() and dram_init_banksize()
	 *   to reserve memory for peripheral space after previous update.
	 */
	if (size_mb > (SDRAM_MAX_SIZE >> 20))
		size_mb = (SDRAM_MAX_SIZE >> 20);

	return (size_t)size_mb << 20;
}

int dram_init(void)
{
	struct ram_info ram;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		return ret;
	}
	ret = ram_get_info(dev, &ram);
	if (ret) {
		debug("Cannot get DRAM size: %d\n", ret);
		return ret;
	}
	gd->ram_size = ram.size;
	debug("SDRAM base=%lx, size=%lx\n",
	      (unsigned long)ram.base, (unsigned long)ram.size);

	return 0;
}

ulong board_get_usable_ram_top(ulong total_size)
{
	unsigned long top = CONFIG_SYS_SDRAM_BASE + SDRAM_MAX_SIZE;

	return (gd->ram_top > top) ? top : gd->ram_top;
}
