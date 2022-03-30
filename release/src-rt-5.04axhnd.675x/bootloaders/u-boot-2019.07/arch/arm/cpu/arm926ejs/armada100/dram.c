// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>,
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/armada100.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * ARMADA100 DRAM controller supports upto 8 banks
 * for chip select 0 and 1
 */

/*
 * DDR Memory Control Registers
 * Refer Datasheet Appendix A.17
 */
struct armd1ddr_map_registers {
	u32	cs;	/* Memory Address Map Register -CS */
	u32	pad[3];
};

struct armd1ddr_registers {
	u8	pad[0x100 - 0x000];
	struct armd1ddr_map_registers mmap[2];
};

/*
 * armd1_sdram_base - reads SDRAM Base Address Register
 */
u32 armd1_sdram_base(int chip_sel)
{
	struct armd1ddr_registers *ddr_regs =
		(struct armd1ddr_registers *)ARMD1_DRAM_BASE;
	u32 result = 0;
	u32 CS_valid = 0x01 & readl(&ddr_regs->mmap[chip_sel].cs);

	if (!CS_valid)
		return 0;

	result = readl(&ddr_regs->mmap[chip_sel].cs) & 0xFF800000;
	return result;
}

/*
 * armd1_sdram_size - reads SDRAM size
 */
u32 armd1_sdram_size(int chip_sel)
{
	struct armd1ddr_registers *ddr_regs =
		(struct armd1ddr_registers *)ARMD1_DRAM_BASE;
	u32 result = 0;
	u32 CS_valid = 0x01 & readl(&ddr_regs->mmap[chip_sel].cs);

	if (!CS_valid)
		return 0;

	result = readl(&ddr_regs->mmap[chip_sel].cs);
	result = (result >> 16) & 0xF;
	if (result < 0x7) {
		printf("Unknown DRAM Size\n");
		return -1;
	} else {
		return ((0x8 << (result - 0x7)) * 1024 * 1024);
	}
}

int dram_init(void)
{
	int i;

	gd->ram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		gd->bd->bi_dram[i].start = armd1_sdram_base(i);
		gd->bd->bi_dram[i].size = armd1_sdram_size(i);
		/*
		 * It is assumed that all memory banks are consecutive
		 * and without gaps.
		 * If the gap is found, ram_size will be reported for
		 * consecutive memory only
		 */
		if (gd->bd->bi_dram[i].start != gd->ram_size)
			break;

		gd->ram_size += gd->bd->bi_dram[i].size;

	}

	for (; i < CONFIG_NR_DRAM_BANKS; i++) {
		/* If above loop terminated prematurely, we need to set
		 * remaining banks' start address & size as 0. Otherwise other
		 * u-boot functions and Linux kernel gets wrong values which
		 * could result in crash */
		gd->bd->bi_dram[i].start = 0;
		gd->bd->bi_dram[i].size = 0;
	}
	return 0;
}

/*
 * If this function is not defined here,
 * board.c alters dram bank zero configuration defined above.
 */
int dram_init_banksize(void)
{
	dram_init();

	return 0;
}
