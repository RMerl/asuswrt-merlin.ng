// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014-2016, Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/arch/mc_me_regs.h>
#include "cpu.h"

u32 cpu_mask(void)
{
	return readl(MC_ME_CS);
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)

#define S32V234_IRAM_BASE        0x3e800000UL
#define S32V234_IRAM_SIZE        0x800000UL
#define S32V234_DRAM_BASE1       0x80000000UL
#define S32V234_DRAM_SIZE1       0x40000000UL
#define S32V234_DRAM_BASE2       0xC0000000UL
#define S32V234_DRAM_SIZE2       0x20000000UL
#define S32V234_PERIPH_BASE      0x40000000UL
#define S32V234_PERIPH_SIZE      0x40000000UL

static struct mm_region s32v234_mem_map[] = {
	{
		.virt = S32V234_IRAM_BASE,
		.phys = S32V234_IRAM_BASE,
		.size = S32V234_IRAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		.virt = S32V234_DRAM_BASE1,
		.phys = S32V234_DRAM_BASE1,
		.size = S32V234_DRAM_SIZE1,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		.virt = S32V234_PERIPH_BASE,
		.phys = S32V234_PERIPH_BASE,
		.size = S32V234_PERIPH_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
			 /* TODO: Do we need these? */
			 /* | PTE_BLOCK_PXN | PTE_BLOCK_UXN */
	}, {
		.virt = S32V234_DRAM_BASE2,
		.phys = S32V234_DRAM_BASE2,
		.size = S32V234_DRAM_SIZE2,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) |
			 PTE_BLOCK_OUTER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = s32v234_mem_map;

#endif

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	int numcores;
	u32 mask;

	mask = cpu_mask();
	numcores = hweight32(cpu_mask());

	/* Verify if M4 is deactivated */
	if (mask & 0x1)
		numcores--;

	return numcores;
}

#if defined(CONFIG_ARCH_EARLY_INIT_R)
int arch_early_init_r(void)
{
	int rv;
	asm volatile ("dsb sy");
	rv = fsl_s32v234_wake_seconday_cores();

	if (rv)
		printf("Did not wake secondary cores\n");

	asm volatile ("sev");
	return 0;
}
#endif /* CONFIG_ARCH_EARLY_INIT_R */
