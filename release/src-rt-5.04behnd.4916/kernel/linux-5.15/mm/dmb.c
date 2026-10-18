// SPDX-License-Identifier: GPL-2.0+
/*
 * Designated Movable Block
 */

#define pr_fmt(fmt) "dmb: " fmt

#include <linux/dmb.h>

struct dmb {
	unsigned long start_pfn;
	unsigned long end_pfn;
};

static struct dmb dmb_areas[CONFIG_DMB_COUNT];
static unsigned int dmb_area_count;

int dmb_intersects(unsigned long spfn, unsigned long epfn)
{
	int i;
	struct dmb *dmb;

	if (spfn >= epfn)
		return DMB_DISJOINT;

	for (i = 0; i < dmb_area_count; i++) {
		dmb = &dmb_areas[i];
		if (spfn >= dmb->end_pfn)
			continue;
		if (epfn <= dmb->start_pfn)
			return DMB_DISJOINT;
		if (spfn >= dmb->start_pfn && epfn <= dmb->end_pfn)
			return DMB_INTERSECTS;
		else
			return DMB_MIXED;
	}

	return DMB_DISJOINT;
}
EXPORT_SYMBOL(dmb_intersects);

int __init dmb_reserve(phys_addr_t base, phys_addr_t size,
		       struct dmb **res_dmb)
{
	struct dmb *dmb;

	/* Sanity checks */
	if (!size || !memblock_is_region_reserved(base, size))
		return -EINVAL;

	/* ensure minimal alignment required by mm core */
	if (!IS_ALIGNED(base | size, DMB_MIN_ALIGNMENT_BYTES))
		return -EINVAL;

	if (dmb_area_count == ARRAY_SIZE(dmb_areas)) {
		pr_warn("Not enough slots for DMB reserved regions!\n");
		return -ENOSPC;
	}

	/*
	 * Each reserved area must be initialised later, when more kernel
	 * subsystems (like slab allocator) are available.
	 */
	dmb = &dmb_areas[dmb_area_count++];

	dmb->start_pfn = PFN_DOWN(base);
	dmb->end_pfn = PFN_DOWN(base + size);
	if (res_dmb)
		*res_dmb = dmb;

	memblock_mark_movable(base, size);
	return 0;
}

void __init dmb_init_region(struct memblock_region *region)
{
	unsigned long pfn;
	int i;

	for (pfn = memblock_region_memory_base_pfn(region);
	     pfn < memblock_region_memory_end_pfn(region);
	     pfn += pageblock_nr_pages) {
		struct page *page = pfn_to_page(pfn);

		for (i = 0; i < pageblock_nr_pages; i++)
			set_page_zone(page + i, ZONE_MOVABLE);

		/* free reserved pageblocks to page allocator */
		init_reserved_pageblock(page);
	}
}
