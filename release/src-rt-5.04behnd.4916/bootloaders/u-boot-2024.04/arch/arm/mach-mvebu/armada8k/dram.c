// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define MV_SIP_DRAM_SIZE	0x82000010

u64 a8k_dram_scan_ap_sz(void)
{
	struct pt_regs pregs;

	pregs.regs[0] = MV_SIP_DRAM_SIZE;
	pregs.regs[1] = SOC_REGS_PHY_BASE;
	smc_call(&pregs);

	return pregs.regs[0];
}

int a8k_dram_init_banksize(void)
{
	/*
	 * The firmware (ATF) leaves a 1G whole above the 3G mark for IO
	 * devices. Higher RAM is mapped at 4G.
	 *
	 * Config 2 DRAM banks:
	 * Bank 0 - max size 4G - 1G
	 * Bank 1 - ram size - 4G + 1G
	 */
	phys_size_t max_bank0_size = SZ_4G - SZ_1G;

	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	if (gd->ram_size <= max_bank0_size) {
		gd->bd->bi_dram[0].size = gd->ram_size;
		return 0;
	}

	gd->bd->bi_dram[0].size = max_bank0_size;
	if (CONFIG_NR_DRAM_BANKS > 1) {
		gd->bd->bi_dram[1].start = SZ_4G;
		gd->bd->bi_dram[1].size = gd->ram_size - max_bank0_size;
	}

	return 0;
}
