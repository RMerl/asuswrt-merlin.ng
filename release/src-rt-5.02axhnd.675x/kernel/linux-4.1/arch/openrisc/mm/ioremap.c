/*
 * OpenRISC ioremap.c
 *
 * Linux architectural port borrowing liberally from similar works of
 * others.  All original copyrights apply as per the original source
 * declaration.
 *
 * Modifications for the OpenRISC architecture:
 * Copyright (C) 2003 Matjaz Breskvar <phoenix@bsemi.com>
 * Copyright (C) 2010-2011 Jonas Bonn <jonas@southpole.se>
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 */

#include <linux/vmalloc.h>
#include <linux/io.h>
#include <asm/pgalloc.h>
#include <asm/kmap_types.h>
#include <asm/fixmap.h>
#include <asm/bug.h>
#include <asm/pgtable.h>
#include <linux/sched.h>
#include <asm/tlbflush.h>

extern int mem_init_done;

static unsigned int fixmaps_used __initdata;

/*
 * Remap an arbitrary physical address space into the kernel virtual
 * address space. Needed when the kernel wants to access high addresses
 * directly.
 *
 * NOTE! We need to allow non-page-aligned mappings too: we will obviously
 * have to convert them into an offset in a page-aligned mapping, but the
 * caller shouldn't need to know that small detail.
 */
void __iomem *__init_refok
__ioremap(phys_addr_t addr, unsigned long size, pgprot_t prot)
{
	phys_addr_t p;
	unsigned long v;
	unsigned long offset, last_addr;
	struct vm_struct *area = NULL;

	/* Don't allow wraparound or zero size */
	last_addr = addr + size - 1;
	if (!size || last_addr < addr)
		return NULL;

	/*
	 * Mappings have to be page-aligned
	 */
	offset = addr & ~PAGE_MASK;
	p = addr & PAGE_MASK;
	size = PAGE_ALIGN(last_addr + 1) - p;

	if (likely(mem_init_done)) {
		area = get_vm_area(size, VM_IOREMAP);
		if (!area)
			return NULL;
		v = (unsigned long)area->addr;
	} else {
		if ((fixmaps_used + (size >> PAGE_SHIFT)) > FIX_N_IOREMAPS)
			return NULL;
		v = fix_to_virt(FIX_IOREMAP_BEGIN + fixmaps_used);
		fixmaps_used += (size >> PAGE_SHIFT);
	}

	if (ioremap_page_range(v, v + size, p, prot)) {
		if (likely(mem_init_done))
			vfree(area->addr);
		else
			fixmaps_used -= (size >> PAGE_SHIFT);
		return NULL;
	}

	return (void __iomem *)(offset + (char *)v);
}

void iounmap(void *addr)
{
	/* If the page is from the fixmap pool then we just clear out
	 * the fixmap mapping.
	 */
	if (unlikely((unsigned long)addr > FIXADDR_START)) {
		/* This is a bit broken... we don't really know
		 * how big the area is so it's difficult to know
		 * how many fixed pages to invalidate...
		 * just flush tlb and hope for the best...
		 * consider this a FIXME
		 *
		 * Really we should be clearing out one or more page
		 * table entries for these virtual addresses so that
		 * future references cause a page fault... for now, we
		 * rely on two things:
		 *   i)  this code never gets called on known boards
		 *   ii) invalid accesses to the freed areas aren't made
		 */
		flush_tlb_all();
		return;
	}

	return vfree((void *)(PAGE_MASK & (unsigned long)addr));
}

/**
 * OK, this one's a bit tricky... ioremap can get called before memory is
 * initialized (early serial console does this) and will want to alloc a page
 * for its mapping.  No userspace pages will ever get allocated before memory
 * is initialized so this applies only to kernel pages.  In the event that
 * this is called before memory is initialized we allocate the page using
 * the memblock infrastructure.
 */

pte_t __init_refok *pte_alloc_one_kernel(struct mm_struct *mm,
					 unsigned long address)
{
	pte_t *pte;

	if (likely(mem_init_done)) {
		pte = (pte_t *) __get_free_page(GFP_KERNEL | __GFP_REPEAT);
	} else {
		pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
#if 0
		/* FIXME: use memblock... */
		pte = (pte_t *) __va(memblock_alloc(PAGE_SIZE, PAGE_SIZE));
#endif
	}

	if (pte)
		clear_page(pte);
	return pte;
}
