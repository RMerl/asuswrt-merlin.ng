// SPDX-License-Identifier: GPL-2.0+
/*
 * Passing basic information from SPL to U-Boot proper
 *
 * Copyright 2018 Google, Inc
 */

#include <common.h>
#include <handoff.h>

DECLARE_GLOBAL_DATA_PTR;

void handoff_save_dram(struct spl_handoff *ho)
{
	ho->ram_size = gd->ram_size;
#ifdef CONFIG_NR_DRAM_BANKS
	{
		struct bd_info *bd = gd->bd;
		int i;

		for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			ho->ram_bank[i].start = bd->bi_dram[i].start;
			ho->ram_bank[i].size = bd->bi_dram[i].size;
		}
	}
#endif
}

void handoff_load_dram_size(struct spl_handoff *ho)
{
	gd->ram_size = ho->ram_size;
}

void handoff_load_dram_banks(struct spl_handoff *ho)
{
#ifdef CONFIG_NR_DRAM_BANKS
	{
		struct bd_info *bd = gd->bd;
		int i;

		for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			bd->bi_dram[i].start = ho->ram_bank[i].start;
			bd->bi_dram[i].size = ho->ram_bank[i].size;
		}
	}
#endif
}
