// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <ioports.h>
#include <lmb.h>
#include <asm/io.h>
#include <asm/mp.h>

DECLARE_GLOBAL_DATA_PTR;

int cpu_reset(u32 nr)
{
	/* dummy function so common/cmd_mp.c will build
	 * should be implemented in the future, when cpu_release()
	 * is supported.  Be aware there may be a similiar bug
	 * as exists on MPC85xx w/its PIC having a timing window
	 * associated to resetting the core */
	return 1;
}

int cpu_status(u32 nr)
{
	/* dummy function so common/cmd_mp.c will build */
	return 0;
}

int cpu_disable(u32 nr)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	volatile ccsr_gur_t *gur = &immap->im_gur;

	switch (nr) {
	case 0:
		setbits_be32(&gur->devdisr, MPC86xx_DEVDISR_CPU0);
		break;
	case 1:
		setbits_be32(&gur->devdisr, MPC86xx_DEVDISR_CPU1);
		break;
	default:
		printf("Invalid cpu number for disable %d\n", nr);
		return 1;
	}

	return 0;
}

int is_core_disabled(int nr) {
	immap_t *immap = (immap_t *) CONFIG_SYS_CCSRBAR;
	ccsr_gur_t *gur = &immap->im_gur;
	u32 devdisr = in_be32(&gur->devdisr);

	switch (nr) {
	case 0:
		return (devdisr & MPC86xx_DEVDISR_CPU0);
	case 1:
		return (devdisr & MPC86xx_DEVDISR_CPU1);
	default:
		printf("Invalid cpu number for disable %d\n", nr);
	}

	return 0;
}

int cpu_release(u32 nr, int argc, char * const argv[])
{
	/* dummy function so common/cmd_mp.c will build
	 * should be implemented in the future */
	return 1;
}

u32 determine_mp_bootpg(unsigned int *pagesize)
{
	if (pagesize)
		*pagesize = 4096;

	/* if we have 4G or more of memory, put the boot page at 4Gb-1M */
	if ((u64)gd->ram_size > 0xfffff000)
		return (0xfff00000);

	return (gd->ram_size - (1024 * 1024));
}

void cpu_mp_lmb_reserve(struct lmb *lmb)
{
	u32 bootpg = determine_mp_bootpg(NULL);

	/* tell u-boot we stole a page */
	lmb_reserve(lmb, bootpg, 4096);
}

/*
 * Copy the code for other cpus to execute into an
 * aligned location accessible via BPTR
 */
void setup_mp(void)
{
	extern ulong __secondary_start_page;
	ulong fixup = (ulong)&__secondary_start_page;
	u32 bootpg = determine_mp_bootpg(NULL);
	u32 bootpg_va;

	if (bootpg >= CONFIG_SYS_MAX_DDR_BAT_SIZE) {
		/* We're not covered by the DDR mapping, set up BAT  */
		write_bat(DBAT7, CONFIG_SYS_SCRATCH_VA | BATU_BL_128K |
			  BATU_VS | BATU_VP,
			  bootpg | BATL_PP_RW | BATL_MEMCOHERENCE);
		bootpg_va = CONFIG_SYS_SCRATCH_VA;
	} else {
		bootpg_va = bootpg;
	}

	memcpy((void *)bootpg_va, (void *)fixup, 4096);
	flush_cache(bootpg_va, 4096);

	/* remove the temporary BAT mapping */
	if (bootpg >= CONFIG_SYS_MAX_DDR_BAT_SIZE)
		write_bat(DBAT7, 0, 0);

	/* If the physical location of bootpg is not at fff00000, set BPTR */
	if (bootpg != 0xfff00000)
		out_be32((uint *)(CONFIG_SYS_CCSRBAR + 0x20), 0x80000000 |
			 (bootpg >> 12));
}
