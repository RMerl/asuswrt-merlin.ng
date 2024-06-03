// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Texas Insturments
 */

/*
 * CP15 specific code
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <asm/armv7.h>
#include <linux/compiler.h>

void __weak v7_arch_cp15_set_l2aux_ctrl(u32 l2actlr, u32 cpu_midr,
				     u32 cpu_rev_comb, u32 cpu_variant,
				     u32 cpu_rev)
{
	asm volatile ("mcr p15, 1, %0, c15, c0, 0\n\t" : : "r"(l2actlr));
}

void __weak v7_arch_cp15_set_acr(u32 acr, u32 cpu_midr, u32 cpu_rev_comb,
				 u32 cpu_variant, u32 cpu_rev)
{
	asm volatile ("mcr p15, 0, %0, c1, c0, 1\n\t" : : "r"(acr));
}
