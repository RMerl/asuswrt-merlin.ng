/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#ifdef CONFIG_CPU_V7A
#include "mmu_map_v7.h"
#else
#include <asm/armv8/mmu.h>
#endif

#if !(CONFIG_IS_ENABLED(SYS_DCACHE_OFF))

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CPU_V7A
int __attribute__((section(".data"))) reg_mmu_init = 0;

void arm_init_domains(void)
{
	/* Set the access control to client so AP is checked with tlb entry */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (0x55555555));
}

/* 
 * Override the weak dram_bank_mmu_setup to setup not only dram but
 * entire mmu based on the mem_map list. This function is called by mmu_setup
 */
void dram_bank_mmu_setup(int bank)
{

	bd_t *bd = gd->bd;
	int i, j, section, num_sec;
	uint32_t *page_table = (u32 *)gd->arch.tlb_addr;
	uint32_t pgt_size = gd->arch.tlb_size;
	uint32_t virt;

	/* zereod out the entire table so undefined region is not accessible */
	if (reg_mmu_init == 0)
		memset((void*)page_table, 0x0, pgt_size);

	if (bd && bd->bi_dram[bank].size) {
		for (i = bd->bi_dram[bank].start >> MMU_SECTION_SHIFT;
		     i < (bd->bi_dram[bank].start >> MMU_SECTION_SHIFT) +
			 (bd->bi_dram[bank].size >> MMU_SECTION_SHIFT);
		     i++) {
			set_section_attr(i, i<<MMU_SECTION_SHIFT, SECTION_ATTR_CACHED_MEM);
		}
	}

	if (reg_mmu_init)
		return;

	for (i = 0; mem_map[i].size; i++) {
		section = mem_map[i].phys >> MMU_SECTION_SHIFT;
		num_sec = mem_map[i].size >> MMU_SECTION_SHIFT;
		virt = mem_map[i].virt >> MMU_SECTION_SHIFT;
		for( j = 0; j < num_sec; j++)
			set_section_attr(section+j, (virt+j)<<MMU_SECTION_SHIFT, mem_map[i].attrs);
	}
	reg_mmu_init = 1;

}

#else

static void bcm_setup_pgtables(void)
{
	u64 tlb_addr = gd->arch.tlb_addr;
	u64 tlb_size = gd->arch.tlb_size;

	/* Reset the fill ptr */
	gd->arch.tlb_fillptr = tlb_addr;

	/* Create normal system page tables */
	setup_pgtables();

#if !defined(CONFIG_SPL_BUILD)
	/* Create emergency page tables */
	gd->arch.tlb_size -= (uintptr_t)gd->arch.tlb_fillptr -
			     (uintptr_t)gd->arch.tlb_addr;
	gd->arch.tlb_addr = gd->arch.tlb_fillptr;
	setup_pgtables();
	gd->arch.tlb_emerg = gd->arch.tlb_addr;
#endif

	gd->arch.tlb_addr = tlb_addr;
	gd->arch.tlb_size = tlb_size;
}

void mmu_setup(void)
{
	int el;

	/* Set up page tables only once */
	if (!gd->arch.tlb_fillptr)
		bcm_setup_pgtables();

	el = current_el();
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr, get_tcr(el, NULL, NULL),
			  MEMORY_ATTRIBUTES);

	/* enable the mmu */
	set_sctlr(get_sctlr() | CR_M);
}
#endif
#endif

void enable_caches(void)
{
	icache_enable();
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}

