/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <physmem.h>
#include <asm/cpu.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

/* Large pages are 2MB. */
#define LARGE_PAGE_SIZE ((1 << 20) * 2)

/*
 * Paging data structures.
 */

struct pdpe {
	uint64_t p:1;
	uint64_t mbz_0:2;
	uint64_t pwt:1;
	uint64_t pcd:1;
	uint64_t mbz_1:4;
	uint64_t avl:3;
	uint64_t base:40;
	uint64_t mbz_2:12;
};

typedef struct pdpe pdpt_t[512];

struct pde {
	uint64_t p:1;      /* present */
	uint64_t rw:1;     /* read/write */
	uint64_t us:1;     /* user/supervisor */
	uint64_t pwt:1;    /* page-level writethrough */
	uint64_t pcd:1;    /* page-level cache disable */
	uint64_t a:1;      /* accessed */
	uint64_t d:1;      /* dirty */
	uint64_t ps:1;     /* page size */
	uint64_t g:1;      /* global page */
	uint64_t avl:3;    /* available to software */
	uint64_t pat:1;    /* page-attribute table */
	uint64_t mbz_0:8;  /* must be zero */
	uint64_t base:31;  /* base address */
};

typedef struct pde pdt_t[512];

static pdpt_t pdpt __aligned(4096);
static pdt_t pdts[4] __aligned(4096);

/*
 * Map a virtual address to a physical address and optionally invalidate any
 * old mapping.
 *
 * @param virt		The virtual address to use.
 * @param phys		The physical address to use.
 * @param invlpg	Whether to use invlpg to clear any old mappings.
 */
static void x86_phys_map_page(uintptr_t virt, phys_addr_t phys, int invlpg)
{
	/* Extract the two bit PDPT index and the 9 bit PDT index. */
	uintptr_t pdpt_idx = (virt >> 30) & 0x3;
	uintptr_t pdt_idx = (virt >> 21) & 0x1ff;

	/* Set up a handy pointer to the appropriate PDE. */
	struct pde *pde = &(pdts[pdpt_idx][pdt_idx]);

	memset(pde, 0, sizeof(struct pde));
	pde->p = 1;
	pde->rw = 1;
	pde->us = 1;
	pde->ps = 1;
	pde->base = phys >> 21;

	if (invlpg) {
		/* Flush any stale mapping out of the TLBs. */
		__asm__ __volatile__(
			"invlpg %0\n\t"
			:
			: "m" (*(uint8_t *)virt)
		);
	}
}

/* Identity map the lower 4GB and turn on paging with PAE. */
static void x86_phys_enter_paging(void)
{
	phys_addr_t page_addr;
	unsigned i;

	/* Zero out the page tables. */
	memset(pdpt, 0, sizeof(pdpt));
	memset(pdts, 0, sizeof(pdts));

	/* Set up the PDPT. */
	for (i = 0; i < ARRAY_SIZE(pdts); i++) {
		pdpt[i].p = 1;
		pdpt[i].base = ((uintptr_t)&pdts[i]) >> 12;
	}

	/* Identity map everything up to 4GB. */
	for (page_addr = 0; page_addr < (1ULL << 32);
			page_addr += LARGE_PAGE_SIZE) {
		/* There's no reason to invalidate the TLB with paging off. */
		x86_phys_map_page(page_addr, page_addr, 0);
	}

	cpu_enable_paging_pae((ulong)pdpt);
}

/* Disable paging and PAE mode. */
static void x86_phys_exit_paging(void)
{
	cpu_disable_paging_pae();
}

/*
 * Set physical memory to a particular value when the whole region fits on one
 * page.
 *
 * @param map_addr	The address that starts the physical page.
 * @param offset	How far into that page to start setting a value.
 * @param c		The value to set memory to.
 * @param size		The size in bytes of the area to set.
 */
static void x86_phys_memset_page(phys_addr_t map_addr, uintptr_t offset, int c,
				 unsigned size)
{
	/*
	 * U-Boot should be far away from the beginning of memory, so that's a
	 * good place to map our window on top of.
	 */
	const uintptr_t window = LARGE_PAGE_SIZE;

	/* Make sure the window is below U-Boot. */
	assert(window + LARGE_PAGE_SIZE <
	       gd->relocaddr - CONFIG_SYS_MALLOC_LEN - CONFIG_SYS_STACK_SIZE);
	/* Map the page into the window and then memset the appropriate part. */
	x86_phys_map_page(window, map_addr, 1);
	memset((void *)(window + offset), c, size);
}

/*
 * A physical memory anologue to memset with matching parameters and return
 * value.
 */
phys_addr_t arch_phys_memset(phys_addr_t start, int c, phys_size_t size)
{
	const phys_addr_t max_addr = (phys_addr_t)~(uintptr_t)0;
	const phys_addr_t orig_start = start;

	if (!size)
		return orig_start;

	/* Handle memory below 4GB. */
	if (start <= max_addr) {
		phys_size_t low_size = min(max_addr + 1 - start, size);
		void *start_ptr = (void *)(uintptr_t)start;

		assert(((phys_addr_t)(uintptr_t)start) == start);
		memset(start_ptr, c, low_size);
		start += low_size;
		size -= low_size;
	}

	/* Use paging and PAE to handle memory above 4GB up to 64GB. */
	if (size) {
		phys_addr_t map_addr = start & ~(LARGE_PAGE_SIZE - 1);
		phys_addr_t offset = start - map_addr;

		x86_phys_enter_paging();

		/* Handle the first partial page. */
		if (offset) {
			phys_addr_t end =
				min(map_addr + LARGE_PAGE_SIZE, start + size);
			phys_size_t cur_size = end - start;
			x86_phys_memset_page(map_addr, offset, c, cur_size);
			size -= cur_size;
			map_addr += LARGE_PAGE_SIZE;
		}
		/* Handle the complete pages. */
		while (size > LARGE_PAGE_SIZE) {
			x86_phys_memset_page(map_addr, 0, c, LARGE_PAGE_SIZE);
			size -= LARGE_PAGE_SIZE;
			map_addr += LARGE_PAGE_SIZE;
		}
		/* Handle the last partial page. */
		if (size)
			x86_phys_memset_page(map_addr, 0, c, size);

		x86_phys_exit_paging();
	}
	return orig_start;
}
