// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/armv7.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>

int print_cpuinfo(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;
	unsigned int id, mdlr;

	mdlr = readl(&gcr->mdlr);

	printf("CPU: ");

	switch (mdlr) {
	case POLEG_NPCM750:
		printf("NPCM750 ");
		break;
	case POLEG_NPCM730:
		printf("NPCM730 ");
		break;
	case POLEG_NPCM710:
		printf("NPCM710 ");
		break;
	default:
		printf("NPCM7XX ");
		break;
	}

	id = readl(&gcr->pdid);
	switch (id) {
	case POLEG_Z1:
		printf("Z1 is no supported! @ ");
		break;
	case POLEG_A1:
		printf("A1 @ ");
		break;
	default:
		printf("Unknown\n");
		break;
	}

	return 0;
}

void s_init(void)
{
	/* Invalidate L2 cache in lowlevel_init */
	v7_outer_cache_inval_all();
}

void enable_caches(void)
{
	dcache_enable();
}

void disable_caches(void)
{
	dcache_disable();
}
