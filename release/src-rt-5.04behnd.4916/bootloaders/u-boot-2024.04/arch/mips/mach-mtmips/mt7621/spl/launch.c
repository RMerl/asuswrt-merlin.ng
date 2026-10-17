// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/io.h>
#include <asm/cm.h>
#include <asm/sections.h>
#include <asm/addrspace.h>
#include <asm/mipsmtregs.h>
#include <linux/sizes.h>
#include <time.h>
#include <cpu_func.h>
#include "launch.h"
#include "../mt7621.h"

/* Cluster Power Controller (CPC) offsets */
#define CPC_CL_OTHER			0x2010
#define CPC_CO_CMD			0x4000

/* CPC_CL_OTHER fields */
#define CPC_CL_OTHER_CORENUM_SHIFT	16
#define CPC_CL_OTHER_CORENUM		GENMASK(23, 16)

/* CPC_CO_CMD */
#define PWR_UP				3

#define NUM_CORES			2
#define NUM_CPUS			4
#define WAIT_CPUS_TIMEOUT		4000

static void copy_launch_wait_code(void)
{
	memset((void *)KSEG1, 0, SZ_4K);

	memcpy((void *)KSEG1ADDR(LAUNCH_WAITCODE),
	       &launch_wait_code_start,
	       &launch_wait_code_end - &launch_wait_code_start);

	invalidate_dcache_range(KSEG0, SZ_4K);
}

static void bootup_secondary_core(void)
{
	void __iomem *cpcbase = (void __iomem *)KSEG1ADDR(MIPS_CPC_BASE);
	int i;

	for (i = 1; i < NUM_CORES; i++) {
		writel(i << CPC_CL_OTHER_CORENUM_SHIFT, cpcbase + CPC_CL_OTHER);
		writel(PWR_UP, cpcbase + CPC_CO_CMD);
	}
}

void secondary_cpu_init(void)
{
	void __iomem *sysc = (void __iomem *)KSEG1ADDR(SYSCTL_BASE);
	u32 i, dual_core = 0, cpuready = 1, cpumask = 0x03;
	ulong wait_tick;
	struct cpulaunch_t *c;

	/* Copy LAUNCH wait code used by other VPEs */
	copy_launch_wait_code();

	dual_core = readl(sysc + SYSCTL_CHIP_REV_ID_REG) & CPU_ID;

	if (dual_core) {
		/* Bootup secondary core for MT7621A */
		cpumask = 0x0f;

		/* Make BootROM/TPL redirect Core1's bootup flow to our entry point */
		writel((uintptr_t)_start, sysc + BOOT_SRAM_BASE_REG);

		bootup_secondary_core();
	}

	/* Join the coherent domain */
	join_coherent_domain(dual_core ? 2 : 1);

	/* Bootup Core0/VPE1 */
	boot_vpe1();

	/* Wait for all CPU ready */
	wait_tick = get_timer(0) + WAIT_CPUS_TIMEOUT;

	while (time_before(get_timer(0), wait_tick)) {
		/* CPU0 is obviously ready */
		for (i = 1; i < NUM_CPUS; i++) {
			c = (struct cpulaunch_t *)(KSEG0ADDR(CPULAUNCH) +
						   (i << LOG2CPULAUNCH));

			if (c->flags & LAUNCH_FREADY)
				cpuready |= BIT(i);
		}

		if ((cpuready & cpumask) == cpumask)
			break;
	}
}
