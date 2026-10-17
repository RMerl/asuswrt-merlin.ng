// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>
#include <asm/mach-types.h>
#include "../common/uart.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	char value[32];
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;

	int ramsize = (readl(&gcr->intcr3) >> 8) & 0x7;

	switch (ramsize) {
	case 0:
		gd->ram_size = 0x08000000; /* 128 MB. */
		break;
	case 1:
		gd->ram_size = 0x10000000; /* 256 MB. */
		break;
	case 2:
		gd->ram_size = 0x20000000; /* 512 MB. */
		break;
	case 3:
		gd->ram_size = 0x40000000; /* 1024 MB. */
		break;
	case 4:
		gd->ram_size = 0x80000000; /* 2048 MB. */
		break;

	default:
	break;
	}

	if (gd->ram_size > 0) {
                sprintf(value, "%ldM", (gd->ram_size / 0x100000));
                env_set("mem", value);
        }

	return 0;
}

int last_stage_init(void)
{
	board_set_console();

	return 0;
}
