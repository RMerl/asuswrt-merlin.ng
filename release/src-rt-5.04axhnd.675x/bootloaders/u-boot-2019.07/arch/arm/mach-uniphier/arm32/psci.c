// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/psci.h>
#include <linux/sizes.h>
#include <asm/processor.h>
#include <asm/psci.h>
#include <asm/secure.h>

#include "../debug.h"
#include "../soc-info.h"
#include "arm-mpcore.h"
#include "cache-uniphier.h"

#define UNIPHIER_SMPCTRL_ROM_RSV2	0x59801208

void uniphier_smp_trampoline(void);
void uniphier_smp_trampoline_end(void);
u32 uniphier_smp_booted[CONFIG_ARMV7_PSCI_NR_CPUS];

static int uniphier_get_nr_cpus(void)
{
	switch (uniphier_get_soc_id()) {
	case UNIPHIER_PRO4_ID:
	case UNIPHIER_PRO5_ID:
		return 2;
	case UNIPHIER_PXS2_ID:
	case UNIPHIER_LD6B_ID:
		return 4;
	default:
		return 1;
	}
}

static void uniphier_smp_kick_all_cpus(void)
{
	const u32 target_ways = BIT(0);
	size_t trmp_size;
	u32 trmp_src = (unsigned long)uniphier_smp_trampoline;
	u32 trmp_src_end = (unsigned long)uniphier_smp_trampoline_end;
	u32 trmp_dest, trmp_dest_end;
	int nr_cpus, i;
	int timeout = 1000;

	nr_cpus = uniphier_get_nr_cpus();
	if (nr_cpus == 1)
		return;

	for (i = 0; i < nr_cpus; i++)	/* lock ways for all CPUs */
		uniphier_cache_set_active_ways(i, 0);
	uniphier_cache_inv_way(target_ways);
	uniphier_cache_enable();

	/* copy trampoline code */
	uniphier_cache_prefetch_range(trmp_src, trmp_src_end, target_ways);

	trmp_size = trmp_src_end - trmp_src;

	trmp_dest = trmp_src & (SZ_64K - 1);
	trmp_dest += SZ_1M - SZ_64K * 2;

	trmp_dest_end = trmp_dest + trmp_size;

	uniphier_cache_touch_range(trmp_dest, trmp_dest_end, target_ways);

	writel(trmp_dest, UNIPHIER_SMPCTRL_ROM_RSV2);

	asm("dsb	ishst\n" /* Ensure the write to ROM_RSV2 is visible */
	    "sev"); /* Bring up all secondary CPUs from Boot ROM into U-Boot */

	while (--timeout) {
		int all_booted = 1;

		for (i = 1; i < nr_cpus; i++)
			if (!uniphier_smp_booted[i])
				all_booted = 0;
		if (all_booted)
			break;
		udelay(1);

		/* barrier here because uniphier_smp_booted[] may be updated */
		cpu_relax();
	}

	if (!timeout)
		pr_warn("warning: some of secondary CPUs may not boot\n");

	uniphier_cache_disable();
}

void psci_board_init(void)
{
	unsigned long scu_base;
	u32 scu_ctrl, tmp;

	asm("mrc p15, 4, %0, c15, c0, 0" : "=r" (scu_base));

	scu_ctrl = readl(scu_base + 0x30);
	if (!(scu_ctrl & 1))
		writel(scu_ctrl | 0x1, scu_base + 0x30);

	scu_ctrl = readl(scu_base + SCU_CTRL);
	scu_ctrl |= SCU_ENABLE | SCU_STANDBY_ENABLE;
	writel(scu_ctrl, scu_base + SCU_CTRL);

	tmp = readl(scu_base + SCU_SNSAC);
	tmp |= 0xfff;
	writel(tmp, scu_base + SCU_SNSAC);

	uniphier_smp_kick_all_cpus();
}

void psci_arch_init(void)
{
	u32 actlr;

	asm("mrc p15, 0, %0, c1, c0, 1" : "=r" (actlr));
	actlr |= 0x41;		/* set SMP and FW bits */
	asm("mcr p15, 0, %0, c1, c0, 1" : : "r" (actlr));
}

u32 uniphier_psci_holding_pen_release __secure_data = 0xffffffff;

int __secure psci_cpu_on(u32 function_id, u32 cpuid, u32 entry_point,
			 u32 context_id)
{
	u32 cpu = cpuid & 0xff;

	debug_puts("[U-Boot PSCI]  psci_cpu_on: cpuid=");
	debug_puth(cpuid);
	debug_puts(", entry_point=");
	debug_puth(entry_point);
	debug_puts(", context_id=");
	debug_puth(context_id);
	debug_puts("\n");

	psci_save(cpu, entry_point, context_id);

	/* We assume D-cache is off, so do not call flush_dcache() here */
	uniphier_psci_holding_pen_release = cpu;

	/* Send an event to wake up the secondary CPU. */
	asm("dsb	ishst\n"
	    "sev");

	return PSCI_RET_SUCCESS;
}

void __secure psci_system_reset(u32 function_id)
{
	reset_cpu(0);
}
