// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <dwmmc.h>
#include <malloc.h>
#include <asm/arcregs.h>
#include "axs10x.h"

DECLARE_GLOBAL_DATA_PTR;

#define AXS_MB_CREG	0xE0011000

int board_early_init_f(void)
{
	if (readl((void __iomem *)AXS_MB_CREG + 0x234) & (1 << 28))
		gd->board_type = AXS_MB_V3;
	else
		gd->board_type = AXS_MB_V2;

	return 0;
}

#ifdef CONFIG_ISA_ARCV2

void board_jump_and_run(ulong entry, int zero, int arch, uint params)
{
	void (*kernel_entry)(int zero, int arch, uint params);

	kernel_entry = (void (*)(int, int, uint))entry;

	smp_set_core_boot_addr(entry, -1);
	smp_kick_all_cpus();
	kernel_entry(zero, arch, params);
}

#define RESET_VECTOR_ADDR	0x0

void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
	/* All cores have reset vector pointing to 0 */
	writel(addr, (void __iomem *)RESET_VECTOR_ADDR);

	/* Make sure other cores see written value in memory */
	flush_dcache_all();
}

void smp_kick_all_cpus(void)
{
/* CPU start CREG */
#define AXC003_CREG_CPU_START	0xF0001400
/* Bits positions in CPU start CREG */
#define BITS_START	0
#define BITS_START_MODE	4
#define BITS_CORE_SEL	9

/*
 * In axs103 v1.1 START bits semantics has changed quite a bit.
 * We used to have a generic START bit for all cores selected by CORE_SEL mask.
 * But now we don't touch CORE_SEL at all because we have a dedicated START bit
 * for each core:
 *     bit 0: Core 0 (master)
 *     bit 1: Core 1 (slave)
 */
#define BITS_START_CORE1	1

#define ARCVER_HS38_3_0	0x53

	int core_family = read_aux_reg(ARC_AUX_IDENTITY) & 0xff;
	int cmd = readl((void __iomem *)AXC003_CREG_CPU_START);

	if (core_family < ARCVER_HS38_3_0) {
		cmd |= (1 << BITS_CORE_SEL) | (1 << BITS_START);
		cmd &= ~(1 << BITS_START_MODE);
	} else {
		cmd |= (1 << BITS_START_CORE1);
	}
	writel(cmd, (void __iomem *)AXC003_CREG_CPU_START);
}
#endif

int checkboard(void)
{
	printf("Board: ARC Software Development Platform AXS%s\n",
	     is_isa_arcv2() ? "103" : "101");

	return 0;
};
