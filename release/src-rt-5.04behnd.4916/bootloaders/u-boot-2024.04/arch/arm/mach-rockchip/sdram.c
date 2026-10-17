// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Rockchip Electronics Co., Ltd.
 */

#include <common.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch-rockchip/sdram.h>
#include <dm/uclass-internal.h>

DECLARE_GLOBAL_DATA_PTR;

#define TRUST_PARAMETER_OFFSET    (34 * 1024 * 1024)

struct tos_parameter_t {
	u32 version;
	u32 checksum;
	struct {
		char name[8];
		s64 phy_addr;
		u32 size;
		u32 flags;
	} tee_mem;
	struct {
		char name[8];
		s64 phy_addr;
		u32 size;
		u32 flags;
	} drm_mem;
	s64 reserve[8];
};

int dram_init_banksize(void)
{
	size_t ram_top = (unsigned long)(gd->ram_size + CFG_SYS_SDRAM_BASE);
	size_t top = min((unsigned long)ram_top, (unsigned long)(gd->ram_top));

#ifdef CONFIG_ARM64
	/* Reserve 0x200000 for ATF bl31 */
	gd->bd->bi_dram[0].start = 0x200000;
	gd->bd->bi_dram[0].size = top - gd->bd->bi_dram[0].start;

	/* Add usable memory beyond the blob of space for peripheral near 4GB */
	if (ram_top > SZ_4G && top < SZ_4G) {
		gd->bd->bi_dram[1].start = SZ_4G;
		gd->bd->bi_dram[1].size = ram_top - gd->bd->bi_dram[1].start;
	}
#else
#ifdef CONFIG_SPL_OPTEE_IMAGE
	struct tos_parameter_t *tos_parameter;

	tos_parameter = (struct tos_parameter_t *)(CFG_SYS_SDRAM_BASE +
			TRUST_PARAMETER_OFFSET);

	if (tos_parameter->tee_mem.flags == 1) {
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = tos_parameter->tee_mem.phy_addr
					- CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[1].start = tos_parameter->tee_mem.phy_addr +
					tos_parameter->tee_mem.size;
		gd->bd->bi_dram[1].size = top - gd->bd->bi_dram[1].start;
	} else {
		gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
		gd->bd->bi_dram[0].size = 0x8400000;
		/* Reserve 32M for OPTEE with TA */
		gd->bd->bi_dram[1].start = CFG_SYS_SDRAM_BASE
					+ gd->bd->bi_dram[0].size + 0x2000000;
		gd->bd->bi_dram[1].size = top - gd->bd->bi_dram[1].start;
	}
#else
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = top - gd->bd->bi_dram[0].start;
#endif
#endif

	return 0;
}

size_t rockchip_sdram_size(phys_addr_t reg)
{
	u32 rank, cs0_col, bk, cs0_row, cs1_row, bw, row_3_4;
	size_t chipsize_mb = 0;
	size_t size_mb = 0;
	u32 ch;
	u32 cs1_col = 0;
	u32 bg = 0;
	u32 dbw, dram_type;
	u32 sys_reg2 = readl(reg);
	u32 sys_reg3 = readl(reg + 4);
	u32 ch_num = 1 + ((sys_reg2 >> SYS_REG_NUM_CH_SHIFT)
		       & SYS_REG_NUM_CH_MASK);
	u32 version = (sys_reg3 >> SYS_REG_VERSION_SHIFT) &
		      SYS_REG_VERSION_MASK;

	dram_type = (sys_reg2 >> SYS_REG_DDRTYPE_SHIFT) & SYS_REG_DDRTYPE_MASK;
	if (version >= 3)
		dram_type |= ((sys_reg3 >> SYS_REG_EXTEND_DDRTYPE_SHIFT) &
			      SYS_REG_EXTEND_DDRTYPE_MASK) << 3;
	debug("%s %x %x\n", __func__, (u32)reg, sys_reg2);
	debug("%s %x %x\n", __func__, (u32)reg + 4, sys_reg3);
	for (ch = 0; ch < ch_num; ch++) {
		rank = 1 + (sys_reg2 >> SYS_REG_RANK_SHIFT(ch) &
			SYS_REG_RANK_MASK);
		cs0_col = 9 + (sys_reg2 >> SYS_REG_COL_SHIFT(ch) &
			  SYS_REG_COL_MASK);
		cs1_col = cs0_col;
		if (dram_type == LPDDR5)
			/* LPDDR5: 0:8bank(bk=3), 1:16bank(bk=4) */
			bk = 3 + ((sys_reg2 >> SYS_REG_BK_SHIFT(ch)) &
			SYS_REG_BK_MASK);
		else
			/* Other: 0:8bank(bk=3), 1:4bank(bk=2) */
			bk = 3 - ((sys_reg2 >> SYS_REG_BK_SHIFT(ch)) &
			SYS_REG_BK_MASK);
		if (version >= 2) {
			cs1_col = 9 + (sys_reg3 >> SYS_REG_CS1_COL_SHIFT(ch) &
				  SYS_REG_CS1_COL_MASK);
			if (((sys_reg3 >> SYS_REG_EXTEND_CS0_ROW_SHIFT(ch) &
			    SYS_REG_EXTEND_CS0_ROW_MASK) << 2) + (sys_reg2 >>
			    SYS_REG_CS0_ROW_SHIFT(ch) &
			    SYS_REG_CS0_ROW_MASK) == 7)
				cs0_row = 12;
			else
				cs0_row = 13 + (sys_reg2 >>
					  SYS_REG_CS0_ROW_SHIFT(ch) &
					  SYS_REG_CS0_ROW_MASK) +
					  ((sys_reg3 >>
					  SYS_REG_EXTEND_CS0_ROW_SHIFT(ch) &
					  SYS_REG_EXTEND_CS0_ROW_MASK) << 2);
			if (((sys_reg3 >> SYS_REG_EXTEND_CS1_ROW_SHIFT(ch) &
			    SYS_REG_EXTEND_CS1_ROW_MASK) << 2) + (sys_reg2 >>
			    SYS_REG_CS1_ROW_SHIFT(ch) &
			    SYS_REG_CS1_ROW_MASK) == 7)
				cs1_row = 12;
			else
				cs1_row = 13 + (sys_reg2 >>
					  SYS_REG_CS1_ROW_SHIFT(ch) &
					  SYS_REG_CS1_ROW_MASK) +
					  ((sys_reg3 >>
					  SYS_REG_EXTEND_CS1_ROW_SHIFT(ch) &
					  SYS_REG_EXTEND_CS1_ROW_MASK) << 2);
		} else {
			cs0_row = 13 + (sys_reg2 >> SYS_REG_CS0_ROW_SHIFT(ch) &
				SYS_REG_CS0_ROW_MASK);
			cs1_row = 13 + (sys_reg2 >> SYS_REG_CS1_ROW_SHIFT(ch) &
				SYS_REG_CS1_ROW_MASK);
		}
		bw = (2 >> ((sys_reg2 >> SYS_REG_BW_SHIFT(ch)) &
			SYS_REG_BW_MASK));
		row_3_4 = sys_reg2 >> SYS_REG_ROW_3_4_SHIFT(ch) &
			SYS_REG_ROW_3_4_MASK;
		if (dram_type == DDR4) {
			dbw = (sys_reg2 >> SYS_REG_DBW_SHIFT(ch)) &
				SYS_REG_DBW_MASK;
			bg = (dbw == 2) ? 2 : 1;
		}
		chipsize_mb = (1 << (cs0_row + cs0_col + bk + bg + bw - 20));

		if (rank > 1)
			chipsize_mb += chipsize_mb >> ((cs0_row - cs1_row) +
				       (cs0_col - cs1_col));
		if (row_3_4)
			chipsize_mb = chipsize_mb * 3 / 4;
		size_mb += chipsize_mb;
		if (rank > 1)
			debug("rank %d cs0_col %d cs1_col %d bk %d cs0_row %d\
			       cs1_row %d bw %d row_3_4 %d\n",
			       rank, cs0_col, cs1_col, bk, cs0_row,
			       cs1_row, bw, row_3_4);
		else
			debug("rank %d cs0_col %d bk %d cs0_row %d\
			       bw %d row_3_4 %d\n",
			       rank, cs0_col, bk, cs0_row,
			       bw, row_3_4);
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
	if (!IS_ENABLED(CONFIG_ARM64) && size_mb > (SDRAM_MAX_SIZE >> 20))
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

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	unsigned long top = CFG_SYS_SDRAM_BASE + SDRAM_MAX_SIZE;

	return (gd->ram_top > top) ? top : gd->ram_top;
}
