/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __DMB_H__
#define __DMB_H__

#include <linux/memblock.h>

/*
 * the buddy -- especially pageblock merging and alloc_contig_range()
 * -- can deal with only some pageblocks of a higher-order page being
 *  MIGRATE_MOVABLE, we can use pageblock_nr_pages.
 */
#define DMB_MIN_ALIGNMENT_PAGES pageblock_nr_pages
#define DMB_MIN_ALIGNMENT_BYTES (PAGE_SIZE * DMB_MIN_ALIGNMENT_PAGES)

enum {
	DMB_DISJOINT = 0,
	DMB_INTERSECTS,
	DMB_MIXED,
};

struct dmb;

extern int dmb_intersects(unsigned long spfn, unsigned long epfn);

extern int dmb_reserve(phys_addr_t base, phys_addr_t size,
		       struct dmb **res_dmb);
extern void dmb_init_region(struct memblock_region *region);

#endif
