#if defined(CONFIG_BCM_KF_ARM_KASAN)
/*
 * This file contains kasan initialization code for ARM.
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 * Author: Andrey Ryabinin <ryabinin.a.a@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kasan.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/bootmem.h>
#include <linux/start_kernel.h>
#include <asm/cputype.h>
#include <asm/highmem.h>
#include <asm/mach/map.h>
#include <asm/memory.h>
#include <asm/page.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/procinfo.h>
#include <asm/proc-fns.h>
#include <asm/tlbflush.h>
#include <asm/cp15.h>
#include <asm/dma.h>
#include <linux/sched/task.h>

#include "mm.h"

static pgd_t tmp_pgd_table[PTRS_PER_PGD] __initdata __aligned(1ULL << 14);

pmd_t tmp_pmd_table[PTRS_PER_PMD] __page_aligned_bss;

static __init void *kasan_alloc_block(size_t size, int node)
{
	return memblock_virt_alloc_try_nid(size, size, __pa(MAX_DMA_ADDRESS),
					BOOTMEM_ALLOC_ACCESSIBLE, node);
}

static void __init kasan_early_pmd_populate(unsigned long start,
					unsigned long end, pud_t *pud)
{
	unsigned long addr;
	unsigned long next;
	pmd_t *pmd;

	pmd = pmd_offset(pud, start);
	for (addr = start; addr < end;) {
		pmd_populate_kernel(&init_mm, pmd, kasan_zero_pte);
		next = pmd_addr_end(addr, end);
		addr = next;
		flush_pmd_entry(pmd);
		pmd++;
	}
}

static void __init kasan_early_pud_populate(unsigned long start,
				unsigned long end, pgd_t *pgd)
{
	unsigned long addr;
	unsigned long next;
	pud_t *pud;

	pud = pud_offset(pgd, start);
	for (addr = start; addr < end;) {
		next = pud_addr_end(addr, end);
		kasan_early_pmd_populate(addr, next, pud);
		addr = next;
		pud++;
	}
}

void __init kasan_map_early_shadow(pgd_t *pgdp)
{
	int i;
	unsigned long start = KASAN_SHADOW_START;
	unsigned long end = KASAN_SHADOW_END;
	unsigned long addr;
	unsigned long next;
	pgd_t *pgd;

	for (i = 0; i < PTRS_PER_PTE; i++)
		set_pte_at(&init_mm, KASAN_SHADOW_START + i*PAGE_SIZE,
			&kasan_zero_pte[i], pfn_pte(
				virt_to_pfn(kasan_zero_page),
				__pgprot(_L_PTE_DEFAULT | L_PTE_DIRTY
					| L_PTE_XN)));

	pgd = pgd_offset_k(start);
	for (addr = start; addr < end;) {
		next = pgd_addr_end(addr, end);
		kasan_early_pud_populate(addr, next, pgd);
		addr = next;
		pgd++;
	}
}

extern struct proc_info_list *lookup_processor_type(unsigned int);

void __init kasan_early_init(void)
{
	struct proc_info_list *list;

	/*
	 * locate processor in the list of supported processor
	 * types.  The linker builds this table for us from the
	 * entries in arch/arm/mm/proc-*.S
	 */
	list = lookup_processor_type(read_cpuid_id());
	if (list) {
#ifdef MULTI_CPU
		processor = *list->proc;
#endif
	}

	BUILD_BUG_ON((KASAN_SHADOW_END - (1UL << 29)) != KASAN_SHADOW_OFFSET);
	kasan_map_early_shadow(swapper_pg_dir);
}

static void __init clear_pgds(unsigned long start,
			unsigned long end)
{
	for (; start && start < end; start += PMD_SIZE)
		pmd_clear(pmd_off_k(start));
}

pte_t * __init kasan_pte_populate(pmd_t *pmd, unsigned long addr, int node)
{
	pte_t *pte = pte_offset_kernel(pmd, addr);

	if (pte_none(*pte)) {
		pte_t entry;
		void *p = kasan_alloc_block(PAGE_SIZE, node);

		if (!p)
			return NULL;
		entry = pfn_pte(virt_to_pfn(p),
			__pgprot(pgprot_val(PAGE_KERNEL)));
		set_pte_at(&init_mm, addr, pte, entry);
	}
	return pte;
}

pmd_t * __init kasan_pmd_populate(pud_t *pud, unsigned long addr, int node)
{
	pmd_t *pmd = pmd_offset(pud, addr);

	if (pmd_none(*pmd)) {
		void *p = kasan_alloc_block(PAGE_SIZE, node);

		if (!p)
			return NULL;
		pmd_populate_kernel(&init_mm, pmd, p);
	}
	return pmd;
}

pud_t * __init kasan_pud_populate(pgd_t *pgd, unsigned long addr, int node)
{
	pud_t *pud = pud_offset(pgd, addr);

	if (pud_none(*pud)) {
		void *p = kasan_alloc_block(PAGE_SIZE, node);

		if (!p)
			return NULL;
		pr_err("populating pud addr %lx\n", addr);
		pud_populate(&init_mm, pud, p);
	}
	return pud;
}

pgd_t * __init kasan_pgd_populate(unsigned long addr, int node)
{
	pgd_t *pgd = pgd_offset_k(addr);

	if (pgd_none(*pgd)) {
		void *p = kasan_alloc_block(PAGE_SIZE, node);

		if (!p)
			return NULL;
		pgd_populate(&init_mm, pgd, p);
	}
	return pgd;
}

static int __init create_mapping(unsigned long start, unsigned long end,
				int node)
{
	unsigned long addr = start;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pr_info("populating shadow for %lx, %lx\n", start, end);

	for (; addr < end; addr += PAGE_SIZE) {
		pgd = kasan_pgd_populate(addr, node);
		if (!pgd)
			return -ENOMEM;

		pud = kasan_pud_populate(pgd, addr, node);
		if (!pud)
			return -ENOMEM;

		pmd = kasan_pmd_populate(pud, addr, node);
		if (!pmd)
			return -ENOMEM;

		pte = kasan_pte_populate(pmd, addr, node);
		if (!pte)
			return -ENOMEM;
	}
	return 0;
}


void __init kasan_init(void)
{
	struct memblock_region *reg;
	u64 orig_ttbr0;
	int i;

	/*
	 * We are going to perform proper setup of shadow memory.
	 * At first we should unmap early shadow (clear_pgds() call bellow).
	 * However, instrumented code couldn't execute without shadow memory.
	 * tmp_pgd_table and tmp_pmd_table used to keep early shadow mapped
	 * until full shadow setup will be finished.
	 */
	orig_ttbr0 = get_ttbr0();

#ifdef CONFIG_ARM_LPAE
	memcpy(tmp_pmd_table,
		pgd_page_vaddr(*pgd_offset_k(KASAN_SHADOW_START)),
		sizeof(tmp_pmd_table));
	memcpy(tmp_pgd_table, swapper_pg_dir, sizeof(tmp_pgd_table));
	set_pgd(&tmp_pgd_table[pgd_index(KASAN_SHADOW_START)],
		__pgd(__pa(tmp_pmd_table) | PMD_TYPE_TABLE | L_PGD_SWAPPER));
	set_ttbr0(__pa(tmp_pgd_table));
#else
	memcpy(tmp_pgd_table, swapper_pg_dir, sizeof(tmp_pgd_table));
	set_ttbr0((u64)__pa(tmp_pgd_table));
#endif
	flush_cache_all();
	local_flush_bp_all();
	local_flush_tlb_all();

	clear_pgds(KASAN_SHADOW_START, KASAN_SHADOW_END);

	kasan_populate_zero_shadow(kasan_mem_to_shadow((void *)VMALLOC_START),
				    kasan_mem_to_shadow((void *)-1UL) + 1);

	for_each_memblock(memory, reg) {
		void *start = __va(reg->base);
		void *end = __va(reg->base + reg->size);

		if (reg->base + reg->size > arm_lowmem_limit)
			end = __va(arm_lowmem_limit);
		if (start >= end)
			break;

		create_mapping((unsigned long)kasan_mem_to_shadow(start),
			(unsigned long)kasan_mem_to_shadow(end),
			NUMA_NO_NODE);
	}

	/*1.the module's global variable is in MODULES_VADDR ~ MODULES_END,
	 *  so we need mapping.
	 *2.PKMAP_BASE ~ PKMAP_BASE+PMD_SIZE's shadow and MODULES_VADDR
	 *  ~ MODULES_END's shadow is in the same PMD_SIZE, so we cant
	 *  use kasan_populate_zero_shadow.
	 */
	create_mapping(
		(unsigned long)kasan_mem_to_shadow((void *)MODULES_VADDR),

		(unsigned long)kasan_mem_to_shadow((void *)(PKMAP_BASE +
							PMD_SIZE)),
		NUMA_NO_NODE);

	/*
	 * KAsan may reuse the contents of kasan_zero_pte directly, so
	 * we should make sure that it maps the zero page read-only.
	 */
	for (i = 0; i < PTRS_PER_PTE; i++)
		set_pte_at(&init_mm, KASAN_SHADOW_START + i*PAGE_SIZE,
			&kasan_zero_pte[i],
			pfn_pte(virt_to_pfn(kasan_zero_page),
				__pgprot(pgprot_val(PAGE_KERNEL)
					| L_PTE_RDONLY)));
	memset(kasan_zero_page, 0, PAGE_SIZE);
	set_ttbr0(orig_ttbr0);
	flush_cache_all();
	local_flush_bp_all();
	local_flush_tlb_all();
	pr_info("Kernel address sanitizer initialized\n");
	init_task.kasan_depth = 0;
}
#endif
