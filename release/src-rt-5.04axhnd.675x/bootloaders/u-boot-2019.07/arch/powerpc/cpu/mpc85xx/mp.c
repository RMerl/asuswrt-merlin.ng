// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/processor.h>
#include <ioports.h>
#include <lmb.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;
u32 fsl_ddr_get_intl3r(void);

extern u32 __spin_table[];

u32 get_my_id()
{
	return mfspr(SPRN_PIR);
}

/*
 * Determine if U-Boot should keep secondary cores in reset, or let them out
 * of reset and hold them in a spinloop
 */
int hold_cores_in_reset(int verbose)
{
	/* Default to no, overridden by 'y', 'yes', 'Y', 'Yes', or '1' */
	if (env_get_yesno("mp_holdoff") == 1) {
		if (verbose) {
			puts("Secondary cores are being held in reset.\n");
			puts("See 'mp_holdoff' environment variable\n");
		}

		return 1;
	}

	return 0;
}

int cpu_reset(u32 nr)
{
	volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC8xxx_PIC_ADDR);
	out_be32(&pic->pir, 1 << nr);
	/* the dummy read works around an errata on early 85xx MP PICs */
	(void)in_be32(&pic->pir);
	out_be32(&pic->pir, 0x0);

	return 0;
}

int cpu_status(u32 nr)
{
	u32 *table, id = get_my_id();

	if (hold_cores_in_reset(1))
		return 0;

	if (nr == id) {
		table = (u32 *)&__spin_table;
		printf("table base @ 0x%p\n", table);
	} else if (is_core_disabled(nr)) {
		puts("Disabled\n");
	} else {
		table = (u32 *)&__spin_table + nr * NUM_BOOT_ENTRY;
		printf("Running on cpu %d\n", id);
		printf("\n");
		printf("table @ 0x%p\n", table);
		printf("   addr - 0x%08x\n", table[BOOT_ENTRY_ADDR_LOWER]);
		printf("   r3   - 0x%08x\n", table[BOOT_ENTRY_R3_LOWER]);
		printf("   pir  - 0x%08x\n", table[BOOT_ENTRY_PIR]);
	}

	return 0;
}

#ifdef CONFIG_FSL_CORENET
int cpu_disable(u32 nr)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->coredisrl, 1 << nr);

	return 0;
}

int is_core_disabled(int nr) {
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 coredisrl = in_be32(&gur->coredisrl);

	return (coredisrl & (1 << nr));
}
#else
int cpu_disable(u32 nr)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	switch (nr) {
	case 0:
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_CPU0);
		break;
	case 1:
		setbits_be32(&gur->devdisr, MPC85xx_DEVDISR_CPU1);
		break;
	default:
		printf("Invalid cpu number for disable %d\n", nr);
		return 1;
	}

	return 0;
}

int is_core_disabled(int nr) {
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr = in_be32(&gur->devdisr);

	switch (nr) {
	case 0:
		return (devdisr & MPC85xx_DEVDISR_CPU0);
	case 1:
		return (devdisr & MPC85xx_DEVDISR_CPU1);
	default:
		printf("Invalid cpu number for disable %d\n", nr);
	}

	return 0;
}
#endif

static u8 boot_entry_map[4] = {
	0,
	BOOT_ENTRY_PIR,
	BOOT_ENTRY_R3_LOWER,
};

int cpu_release(u32 nr, int argc, char * const argv[])
{
	u32 i, val, *table = (u32 *)&__spin_table + nr * NUM_BOOT_ENTRY;
	u64 boot_addr;

	if (hold_cores_in_reset(1))
		return 0;

	if (nr == get_my_id()) {
		printf("Invalid to release the boot core.\n\n");
		return 1;
	}

	if (argc != 4) {
		printf("Invalid number of arguments to release.\n\n");
		return 1;
	}

	boot_addr = simple_strtoull(argv[0], NULL, 16);

	/* handle pir, r3 */
	for (i = 1; i < 3; i++) {
		if (argv[i][0] != '-') {
			u8 entry = boot_entry_map[i];
			val = simple_strtoul(argv[i], NULL, 16);
			table[entry] = val;
		}
	}

	table[BOOT_ENTRY_ADDR_UPPER] = (u32)(boot_addr >> 32);

	/* ensure all table updates complete before final address write */
	eieio();

	table[BOOT_ENTRY_ADDR_LOWER] = (u32)(boot_addr & 0xffffffff);

	return 0;
}

u32 determine_mp_bootpg(unsigned int *pagesize)
{
	u32 bootpg;
#ifdef CONFIG_SYS_FSL_ERRATUM_A004468
	u32 svr = get_svr();
	u32 granule_size, check;
	struct law_entry e;
#endif


	/* use last 4K of mapped memory */
	bootpg = ((gd->ram_size > CONFIG_MAX_MEM_MAPPED) ?
		CONFIG_MAX_MEM_MAPPED : gd->ram_size) +
		CONFIG_SYS_SDRAM_BASE - 4096;
	if (pagesize)
		*pagesize = 4096;

#ifdef CONFIG_SYS_FSL_ERRATUM_A004468
/*
 * Erratum A004468 has two parts. The 3-way interleaving applies to T4240,
 * to be fixed in rev 2.0. The 2-way interleaving applies to many SoCs. But
 * the way boot page chosen in u-boot avoids hitting this erratum. So only
 * thw workaround for 3-way interleaving is needed.
 *
 * To make sure boot page translation works with 3-Way DDR interleaving
 * enforce a check for the following constrains
 * 8K granule size requires BRSIZE=8K and
 *    bootpg >> log2(BRSIZE) %3 == 1
 * 4K and 1K granule size requires BRSIZE=4K and
 *    bootpg >> log2(BRSIZE) %3 == 0
 */
	if (SVR_SOC_VER(svr) == SVR_T4240 && SVR_MAJ(svr) < 2) {
		e = find_law(bootpg);
		switch (e.trgt_id) {
		case LAW_TRGT_IF_DDR_INTLV_123:
			granule_size = fsl_ddr_get_intl3r() & 0x1f;
			if (granule_size == FSL_DDR_3WAY_8KB_INTERLEAVING) {
				if (pagesize)
					*pagesize = 8192;
				bootpg &= 0xffffe000;	/* align to 8KB */
				check = bootpg >> 13;
				while ((check % 3) != 1)
					check--;
				bootpg = check << 13;
				debug("Boot page (8K) at 0x%08x\n", bootpg);
				break;
			} else {
				bootpg &= 0xfffff000;	/* align to 4KB */
				check = bootpg >> 12;
				while ((check % 3) != 0)
					check--;
				bootpg = check << 12;
				debug("Boot page (4K) at 0x%08x\n", bootpg);
			}
				break;
		default:
			break;
		}
	}
#endif /* CONFIG_SYS_FSL_ERRATUM_A004468 */

	return bootpg;
}

phys_addr_t get_spin_phys_addr(void)
{
	return virt_to_phys(&__spin_table);
}

#ifdef CONFIG_FSL_CORENET
static void plat_mp_up(unsigned long bootpg, unsigned int pagesize)
{
	u32 cpu_up_mask, whoami, brsize = LAW_SIZE_4K;
	u32 *table = (u32 *)&__spin_table;
	volatile ccsr_gur_t *gur;
	volatile ccsr_local_t *ccm;
	volatile ccsr_rcpm_t *rcpm;
	volatile ccsr_pic_t *pic;
	int timeout = 10;
	u32 mask = cpu_mask();
	struct law_entry e;

	gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	ccm = (void *)(CONFIG_SYS_FSL_CORENET_CCM_ADDR);
	rcpm = (void *)(CONFIG_SYS_FSL_CORENET_RCPM_ADDR);
	pic = (void *)(CONFIG_SYS_MPC8xxx_PIC_ADDR);

	whoami = in_be32(&pic->whoami);
	cpu_up_mask = 1 << whoami;
	out_be32(&ccm->bstrl, bootpg);

	e = find_law(bootpg);
	/* pagesize is only 4K or 8K */
	if (pagesize == 8192)
		brsize = LAW_SIZE_8K;
	out_be32(&ccm->bstrar, LAW_EN | e.trgt_id << 20 | brsize);
	debug("BRSIZE is 0x%x\n", brsize);

	/* readback to sync write */
	in_be32(&ccm->bstrar);

	/* disable time base at the platform */
	out_be32(&rcpm->ctbenrl, cpu_up_mask);

	out_be32(&gur->brrl, mask);

	/* wait for everyone */
	while (timeout) {
		unsigned int i, cpu, nr_cpus = cpu_numcores();

		for_each_cpu(i, cpu, nr_cpus, mask) {
			if (table[cpu * NUM_BOOT_ENTRY + BOOT_ENTRY_ADDR_LOWER])
				cpu_up_mask |= (1 << cpu);
		}

		if ((cpu_up_mask & mask) == mask)
			break;

		udelay(100);
		timeout--;
	}

	if (timeout == 0)
		printf("CPU up timeout. CPU up mask is %x should be %x\n",
			cpu_up_mask, mask);

	/* enable time base at the platform */
	out_be32(&rcpm->ctbenrl, 0);

	/* readback to sync write */
	in_be32(&rcpm->ctbenrl);

	mtspr(SPRN_TBWU, 0);
	mtspr(SPRN_TBWL, 0);

	out_be32(&rcpm->ctbenrl, mask);

#ifdef CONFIG_MPC8xxx_DISABLE_BPTR
	/*
	 * Disabling Boot Page Translation allows the memory region 0xfffff000
	 * to 0xffffffff to be used normally.  Leaving Boot Page Translation
	 * enabled remaps 0xfffff000 to SDRAM which makes that memory region
	 * unusable for normal operation but it does allow OSes to easily
	 * reset a processor core to put it back into U-Boot's spinloop.
	 */
	clrbits_be32(&ccm->bstrar, LAW_EN);
#endif
}
#else
static void plat_mp_up(unsigned long bootpg, unsigned int pagesize)
{
	u32 up, cpu_up_mask, whoami;
	u32 *table = (u32 *)&__spin_table;
	volatile u32 bpcr;
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	volatile ccsr_pic_t *pic = (void *)(CONFIG_SYS_MPC8xxx_PIC_ADDR);
	u32 devdisr;
	int timeout = 10;

	whoami = in_be32(&pic->whoami);
	out_be32(&ecm->bptr, 0x80000000 | (bootpg >> 12));

	/* disable time base at the platform */
	devdisr = in_be32(&gur->devdisr);
	if (whoami)
		devdisr |= MPC85xx_DEVDISR_TB0;
	else
		devdisr |= MPC85xx_DEVDISR_TB1;
	out_be32(&gur->devdisr, devdisr);

	/* release the hounds */
	up = ((1 << cpu_numcores()) - 1);
	bpcr = in_be32(&ecm->eebpcr);
	bpcr |= (up << 24);
	out_be32(&ecm->eebpcr, bpcr);
	asm("sync; isync; msync");

	cpu_up_mask = 1 << whoami;
	/* wait for everyone */
	while (timeout) {
		int i;
		for (i = 0; i < cpu_numcores(); i++) {
			if (table[i * NUM_BOOT_ENTRY + BOOT_ENTRY_ADDR_LOWER])
				cpu_up_mask |= (1 << i);
		};

		if ((cpu_up_mask & up) == up)
			break;

		udelay(100);
		timeout--;
	}

	if (timeout == 0)
		printf("CPU up timeout. CPU up mask is %x should be %x\n",
			cpu_up_mask, up);

	/* enable time base at the platform */
	if (whoami)
		devdisr |= MPC85xx_DEVDISR_TB1;
	else
		devdisr |= MPC85xx_DEVDISR_TB0;
	out_be32(&gur->devdisr, devdisr);

	/* readback to sync write */
	in_be32(&gur->devdisr);

	mtspr(SPRN_TBWU, 0);
	mtspr(SPRN_TBWL, 0);

	devdisr &= ~(MPC85xx_DEVDISR_TB0 | MPC85xx_DEVDISR_TB1);
	out_be32(&gur->devdisr, devdisr);

#ifdef CONFIG_MPC8xxx_DISABLE_BPTR
	/*
	 * Disabling Boot Page Translation allows the memory region 0xfffff000
	 * to 0xffffffff to be used normally.  Leaving Boot Page Translation
	 * enabled remaps 0xfffff000 to SDRAM which makes that memory region
	 * unusable for normal operation but it does allow OSes to easily
	 * reset a processor core to put it back into U-Boot's spinloop.
	 */
	clrbits_be32(&ecm->bptr, 0x80000000);
#endif
}
#endif

void cpu_mp_lmb_reserve(struct lmb *lmb)
{
	u32 bootpg = determine_mp_bootpg(NULL);

	lmb_reserve(lmb, bootpg, 4096);
}

void setup_mp(void)
{
	extern u32 __secondary_start_page;
	extern u32 __bootpg_addr, __spin_table_addr, __second_half_boot_page;

	int i;
	ulong fixup = (u32)&__secondary_start_page;
	u32 bootpg, bootpg_map, pagesize;

	bootpg = determine_mp_bootpg(&pagesize);

	/*
	 * pagesize is only 4K or 8K
	 * we only use the last 4K of boot page
	 * bootpg_map saves the address for the boot page
	 * 8K is used for the workaround of 3-way DDR interleaving
	 */

	bootpg_map = bootpg;

	if (pagesize == 8192)
		bootpg += 4096;	/* use 2nd half */

	/* Some OSes expect secondary cores to be held in reset */
	if (hold_cores_in_reset(0))
		return;

	/*
	 * Store the bootpg's cache-able half address for use by secondary
	 * CPU cores to continue to boot
	 */
	__bootpg_addr = (u32)virt_to_phys(&__second_half_boot_page);

	/* Store spin table's physical address for use by secondary cores */
	__spin_table_addr = (u32)get_spin_phys_addr();

	/* flush bootpg it before copying invalidate any staled cacheline */
	flush_cache(bootpg, 4096);

	/* look for the tlb covering the reset page, there better be one */
	i = find_tlb_idx((void *)CONFIG_BPTR_VIRT_ADDR, 1);

	/* we found a match */
	if (i != -1) {
		/* map reset page to bootpg so we can copy code there */
		disable_tlb(i);

		set_tlb(1, CONFIG_BPTR_VIRT_ADDR, bootpg, /* tlb, epn, rpn */
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G, /* perms, wimge */
			0, i, BOOKE_PAGESZ_4K, 1); /* ts, esel, tsize, iprot */

		memcpy((void *)CONFIG_BPTR_VIRT_ADDR, (void *)fixup, 4096);

		plat_mp_up(bootpg_map, pagesize);
	} else {
		puts("WARNING: No reset page TLB. "
			"Skipping secondary core setup\n");
	}
}
