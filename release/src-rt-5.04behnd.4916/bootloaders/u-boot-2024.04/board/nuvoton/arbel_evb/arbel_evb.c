// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>
#include "../common/uart.h"

#define SR_MII_CTRL_SWR_BIT15	15

#define DRAM_512MB_ECC_SIZE	0x1C000000ULL
#define DRAM_512MB_SIZE		0x20000000ULL
#define DRAM_1GB_ECC_SIZE	0x38000000ULL
#define DRAM_1GB_SIZE		0x40000000ULL
#define DRAM_2GB_ECC_SIZE	0x70000000ULL
#define DRAM_2GB_SIZE		0x80000000ULL
#define DRAM_4GB_ECC_SIZE	0xE0000000ULL
#define DRAM_4GB_SIZE		0x100000000ULL

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;

	/*
	 * get dram active size value from bootblock.
	 * Value sent using scrpad_03 register.
	 * feature available in bootblock 0.0.6 and above.
	 */

	gd->ram_size = readl(&gcr->scrpad_c);

	if (gd->ram_size == 0)
		gd->ram_size = readl(&gcr->scrpad_b);
	else
		gd->ram_size *= 0x100000ULL;

	debug("ram_size: %llx ", gd->ram_size);

	return 0;
}

int dram_init_banksize(void)
{

	gd->bd->bi_dram[0].start = 0;

	switch (gd->ram_size) {
	case DRAM_512MB_ECC_SIZE:
	case DRAM_512MB_SIZE:
	case DRAM_1GB_ECC_SIZE:
	case DRAM_1GB_SIZE:
	case DRAM_2GB_ECC_SIZE:
	case DRAM_2GB_SIZE:
		gd->bd->bi_dram[0].size = gd->ram_size;
		gd->bd->bi_dram[1].start = 0;
		gd->bd->bi_dram[1].size = 0;
		break;
	case DRAM_4GB_ECC_SIZE:
		gd->bd->bi_dram[0].size = DRAM_2GB_SIZE;
		gd->bd->bi_dram[1].start = DRAM_4GB_SIZE;
		gd->bd->bi_dram[1].size = DRAM_2GB_SIZE -
			(DRAM_4GB_SIZE - DRAM_4GB_ECC_SIZE);
		/* use bank0 only */
		gd->ram_size = DRAM_2GB_SIZE;
		break;
	case DRAM_4GB_SIZE:
		gd->bd->bi_dram[0].size = DRAM_2GB_SIZE;
		gd->bd->bi_dram[1].start = DRAM_4GB_SIZE;
		gd->bd->bi_dram[1].size = DRAM_2GB_SIZE;
		/* use bank0 only */
		gd->ram_size = DRAM_2GB_SIZE;
		break;
	default:
		gd->bd->bi_dram[0].size = DRAM_1GB_SIZE;
		gd->bd->bi_dram[1].start = 0;
		gd->bd->bi_dram[1].size = 0;
		gd->ram_size = DRAM_1GB_SIZE;
		break;
	}

	return 0;
}

int last_stage_init(void)
{
	board_set_console();

	return 0;
}
