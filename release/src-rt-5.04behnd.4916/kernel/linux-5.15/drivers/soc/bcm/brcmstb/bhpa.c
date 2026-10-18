/*
 * Copyright © 2023 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */

#include <linux/bitmap.h>
#include <linux/mm.h>   /* for alloc_contig_range */
#include <linux/brcmstb/memory_api.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of_address.h>
#include <linux/of_reserved_mem.h>
#include <linux/seq_file.h> /* seq_print single_open */
#include <linux/slab.h> /* kmalloc */
#include <linux/dmb.h>
#include <asm/io.h>

#include "bhpa.h"

/*  size of single HUGE page allocation block, in bytes */
#define BHPA_BLOCK_MAX (1 * (uint64_t)1024 * 1024 * 1024)
/*  number of HUGE pages in single HUGE page allocation block */
#define BHPA_BLOCK_PAGES ((unsigned int)(BHPA_BLOCK_MAX / BHPA_SIZE))

/* struct bhpa_block - BHPA block structure
 * @node: list head node
 * @base: start address of this block
 * @count: number of pages in the block
 * @free: number of non-busy pages in the block
 * @stats: statistics structure
 * @busy: bitmap of busy pages - allocate or not available
 * @allocated: bitmap of allocate pages
 */
struct bhpa_block {
	struct list_head node;
	phys_addr_t base;
	unsigned int count;
	unsigned int free;
	struct {
		/* number of busy pages */
		unsigned int busy;
		/* number of allocated pages */
		unsigned int allocated;
		/* largest number of allocated pages */
		unsigned int high_allocated;
		/* largest number of 'busy' pages */
		unsigned int high_busy;
		struct {
			unsigned int fast[4];
			unsigned int slow;
		} allocations;
	} stats;
	DECLARE_BITMAP(busy, BHPA_BLOCK_PAGES);
	DECLARE_BITMAP(allocated, BHPA_BLOCK_PAGES);
	unsigned mt;
};

struct bhpa_memc {
	struct list_head blocks;
};

struct bhpa_allocator {
	struct bhpa_memc memc[MAX_BRCMSTB_MEMC];
	struct dentry *debugfs;
};

static DEFINE_MUTEX(bhpa_lock);
static struct bhpa_allocator bhpa_allocator;

static char *ranges = "";
module_param(ranges,charp,0440);

#if IS_ENABLED(CONFIG_BRCMSTB_MEMORY_API)
extern struct bhpa_region bhpa_regions[MAX_BHPA_REGIONS];
extern unsigned int n_bhpa_regions;

static int bhpa_memory_phys_addr_to_memc(phys_addr_t pa)
{
	return brcmstb_memory_phys_addr_to_memc(pa);
}
#else
static struct bhpa_region bhpa_regions[MAX_BHPA_REGIONS];
static unsigned int n_bhpa_regions;

/* Constants used when retrieving memc info */
#define NUM_BUS_RANGES		10
#define BUS_RANGE_ULIMIT_SHIFT	4
#define BUS_RANGE_LLIMIT_SHIFT	4
#define BUS_RANGE_PA_SHIFT	12

enum {
	BUSNUM_MCP0 = 0x4,
	BUSNUM_MCP1 = 0x5,
	BUSNUM_MCP2 = 0x6,
};

/*
 * If the DT nodes are handy, determine which MEMC holds the specified
 * physical address.
 */
static int __memory_phys_addr_to_memc(phys_addr_t pa, void __iomem *base,
				      const char *compat)
{
	const char *bcm7211_biuctrl_match = "brcm,bcm7211-cpu-biu-ctrl";
	const char *bcm72113_biuctrl_match = "brcm,bcm72113-cpu-biu-ctrl";
	int memc = -1;
	int i;

	/* Single MEMC controller with unreadable ULIMIT values as of the A0 */
	if (!strncmp(compat, bcm7211_biuctrl_match,
		     strlen(bcm7211_biuctrl_match)))
		return 0;
	if (!strncmp(compat, bcm72113_biuctrl_match,
		     strlen(bcm72113_biuctrl_match)))
		return 0;

	for (i = 0; i < NUM_BUS_RANGES; i++, base += 8) {
		const u64 ulimit_raw = readl(base);
		const u64 llimit_raw = readl(base + 4);
		const u64 ulimit =
			((ulimit_raw >> BUS_RANGE_ULIMIT_SHIFT)
			 << BUS_RANGE_PA_SHIFT) | 0xfff;
		const u64 llimit = (llimit_raw >> BUS_RANGE_LLIMIT_SHIFT)
				   << BUS_RANGE_PA_SHIFT;
		const u32 busnum = (u32)(ulimit_raw & 0xf);

		if (pa >= llimit && pa <= ulimit) {
			if (busnum >= BUSNUM_MCP0 && busnum <= BUSNUM_MCP2) {
				memc = busnum - BUSNUM_MCP0;
				break;
			}
		}
	}

	return memc;
}

static int bhpa_memory_phys_addr_to_memc(phys_addr_t pa)
{
	int memc = 0;
	struct device_node *np;
	void __iomem *cpubiuctrl;
	const char *compat;

	np = of_find_compatible_node(NULL, NULL, "brcm,brcmstb-cpu-biu-ctrl");
	if (!np)
		return memc;

	compat = of_get_property(np, "compatible", NULL);

	cpubiuctrl = of_iomap(np, 0);
	if (!cpubiuctrl) {
		memc = -ENOMEM;
		goto cleanup;
	}

	memc = __memory_phys_addr_to_memc(pa, cpubiuctrl, compat);
	iounmap(cpubiuctrl);

cleanup:
	of_node_put(np);

	return memc;
}
#endif

static int __init __bhpa_setup(phys_addr_t addr, phys_addr_t size, int memc,
				unsigned mt)
{
	phys_addr_t end = addr + size;
	int i;

	/* Consolidate overlapping regions */
	for (i = 0; i < n_bhpa_regions; i++) {
		if (addr > bhpa_regions[i].addr + bhpa_regions[i].size)
			continue;
		if (end < bhpa_regions[i].addr)
			continue;
		if (memc != bhpa_regions[i].memc || mt != bhpa_regions[i].mt) {
			if (addr == bhpa_regions[i].addr + bhpa_regions[i].size)
				continue;
			if (end == bhpa_regions[i].addr)
				continue;
			pr_warn("overlaping range dropped! %d: %pa-%pa\n",
				memc, &addr, &end);
			return -EINVAL;
		}
		end = max(end, bhpa_regions[i].addr + bhpa_regions[i].size);
		addr = min(addr, bhpa_regions[i].addr);
		bhpa_regions[i].addr = bhpa_regions[--n_bhpa_regions].addr;
		bhpa_regions[i].size = bhpa_regions[n_bhpa_regions].size;
		bhpa_regions[i].mt = bhpa_regions[n_bhpa_regions].mt;
		bhpa_regions[i--].memc = bhpa_regions[n_bhpa_regions].memc;
	}

	if (n_bhpa_regions == MAX_BHPA_REGIONS) {
		pr_warn_once("too many regions, ignoring extras\n");
		return -E2BIG;
	}

	bhpa_regions[n_bhpa_regions].addr = addr;
	bhpa_regions[n_bhpa_regions].size = end - addr;
	bhpa_regions[n_bhpa_regions].memc = memc;
	bhpa_regions[n_bhpa_regions].mt = mt;
	n_bhpa_regions++;

	return 0;
}

static int __init bhpa_parameter_parse_ranges(char *str)
{
	phys_addr_t addr, end, size;

	while (str && *str != '\0') {
		char *orig_str = str;
		int memc;

		end = 0;
		addr = memparse(str, &str);
		if (*str == '@') {
			size = addr;
			addr = memparse(str + 1, &str);
			end = addr + size;
		} else if (*str == '-') {
			end = memparse(str + 1, &str);
		}

		if (*str == ',') {
			str++;
		} else if (*str != '\0') {
			pr_warn("malformed parameter: %s\n", orig_str);
			break;
		}

		addr = ALIGN(addr, BHPA_ALIGN);
		end = ALIGN_DOWN(end, BHPA_ALIGN);
		size = end - addr;

		if (size == 0) {
			pr_info("disabling BHPA memory\n");
			n_bhpa_regions = 0;
			return 0;
		}

		if (addr > end || size < pageblock_nr_pages << PAGE_SHIFT) {
			pr_warn("ignoring invalid range '%s' (too small)\n",
				orig_str);
			continue;
		}

		if (dmb_intersects(PFN_DOWN(addr), PFN_UP(end))
		    != DMB_INTERSECTS) {
			pr_warn("ignoring invalid range '%s' (no DMB)\n",
				orig_str);
			continue;
		}

		memc = bhpa_memory_phys_addr_to_memc(addr);
		if (bhpa_memory_phys_addr_to_memc(end) != memc)
			pr_warn("ignoring invalid range '%s' (split MEMC)\n",
				orig_str);
		else
			__bhpa_setup(addr, size, memc, MIGRATE_MOVABLE);
	}

	return 0;
}

static struct page *bhpa_get_free_range_in_zone(struct zone *zone,
						unsigned long start,
						unsigned long end,
						unsigned int migratetype,
						unsigned int order)
{
	struct page *free_page = NULL;
	unsigned long flags;

	if (!populated_zone(zone))
		return NULL;
	B_LOG_TRACE("free_range: zone:%p %s at %lx",
		    (void *)zone, zone->name, zone->zone_start_pfn);
	for (; order < MAX_ORDER; order++) {
		struct free_area *area = &(zone->free_area[order]);
		struct page *page;

		B_LOG_TRACE("free_range: zone:%p area:%p order:%u migratetype:%u",
			    (void *)zone, (void *)area, order, migratetype);
		spin_lock_irqsave(&zone->lock, flags);
		list_for_each_entry(page, &area->free_list[migratetype], lru) {
			unsigned long pfn;

			B_LOG_TRACE("free_range: zone:%p page:%p",
				    (void *)zone, (void *)page);
			pfn = page_to_pfn(page);
			B_LOG_TRACE("free_range: zone:%p page:%lx..%lx order:%u range:%lx..%lx",
				    (void *)zone, pfn, pfn + (1 << order),
				    order, start, end);
			if (pfn >= start && (pfn + (1 << order)) <= end) {
				free_page = page;
				break;
			}
		}
		spin_unlock_irqrestore(&zone->lock, flags);
		if (free_page)
			break;
	}
	return free_page;
}

static struct page *bhpa_get_free_page_in_range(unsigned long start,
						unsigned long end,
						unsigned int migratetype,
						unsigned int order)
{
	struct page *start_page;
	struct zone *start_zone;

	if (!pfn_valid(start))
		return NULL;

	start_page = pfn_to_page(start);
	start_zone = page_zone(start_page);
	return bhpa_get_free_range_in_zone(start_zone, start, end, migratetype,
					   order);
}

static void bhpa_block_init(struct bhpa_block *block, phys_addr_t base,
			    unsigned int pages, unsigned mt)
{
	if (WARN_ON(pages > BHPA_BLOCK_PAGES))
		pages = BHPA_BLOCK_PAGES;
	block->base = base;
	block->count = pages;
	block->free = pages;
	memset(&block->stats, 0, sizeof(block->stats));
	bitmap_zero(block->allocated, BHPA_BLOCK_PAGES);
	bitmap_zero(block->busy, BHPA_BLOCK_PAGES);
	bitmap_set(block->busy, pages, BHPA_BLOCK_PAGES - pages);
	block->mt = mt;
}

/* bhpa_block_print - Print a BHPA block details
 *
 * @seq: sequence file when called from debugfs
 * block: BHPA block structure to print
 * @memc: memory controller index
 */
static void bhpa_block_print(struct seq_file *seq,
			     const struct bhpa_block *block,
			     unsigned int memc)
{
	unsigned int i;
	char buf[80];
	int buf_off = 0;

	if (seq)
		seq_printf(seq, "MEMC%u %#llx..%#llx(%p) %u/%u/%u/%u (%u/%u) (%u/%u/%u/%u/%u)\n",
			   memc,
			   (unsigned long long)block->base,
			   (unsigned long long)(block->base + block->count *
						(uint64_t)BHPA_SIZE),
			   block, block->count, block->free,
			   block->stats.allocated, block->stats.busy,
			   block->stats.high_allocated, block->stats.high_busy,
			   block->stats.allocations.fast[0],
			   block->stats.allocations.fast[1],
			   block->stats.allocations.fast[2],
			   block->stats.allocations.fast[3],
			   block->stats.allocations.slow);
	else
		pr_info("MEMC%u %#llx..%#llx(%p) %u/%u/%u/%u (%u/%u) (%u/%u/%u/%u/%u)\n",
			   memc,
			   (unsigned long long)block->base,
			   (unsigned long long)(block->base + block->count *
						(uint64_t)BHPA_SIZE),
			   block, block->count, block->free,
			   block->stats.allocated, block->stats.busy,
			   block->stats.high_allocated, block->stats.high_busy,
			   block->stats.allocations.fast[0],
			   block->stats.allocations.fast[1],
			   block->stats.allocations.fast[2],
			   block->stats.allocations.fast[3],
			   block->stats.allocations.slow);

	for (i = 0; i < block->count; i++) {
		int rc;
		int left;
		int busy;
		int allocated;
		char ch;
		phys_addr_t addr;

		if (buf_off == 0) {
			addr = block->base + i * (phys_addr_t)BHPA_SIZE;
			rc = snprintf(buf, sizeof(buf), " %#llx: ",
				      (unsigned long long)addr);
			if (!WARN_ON(rc < 0 || rc > sizeof(buf)))
				buf_off = rc;
		}
		left = sizeof(buf) - buf_off;
		allocated = test_bit(i, block->allocated);
		busy = test_bit(i, block->busy);
		ch = '.';
		if (allocated)
			if (!busy)
				ch = 'X';
			else
				ch = 'A';
		else if (busy)
			ch = 'B';
		rc = snprintf(buf + buf_off, sizeof(buf) - buf_off, "%c", ch);
		if (rc <= 0 || rc >= left) {
			if (seq)
				seq_printf(seq, "%s\n", buf);
			else
				pr_info("%s\n", buf);
			if (!WARN_ON(i == 0))
				i--;
			buf_off = 0;
			continue;
		}
	buf_off += rc;
	}
	if (buf_off) {
		if (seq)
			seq_printf(seq, "%s\n", buf);
		else
			pr_info("%s\n", buf);
	}
}


/* bhpa_block_update_range - Update a BHPA block range
 *
 * @block: BHPA block to update
 * @range: range to supply to the block
 * @p_first_page: pointer to the first page
 * @p_last_page: pointer to the last page
 */
static void bhpa_block_update_range(const struct bhpa_block *block,
				    const struct brcmstb_range *range,
				    unsigned int *p_first_page,
				    unsigned int *p_last_page)
{
	unsigned int first_page = 0;
	unsigned int last_page = block->count;

	if (block->free == 0) {
		last_page = first_page;
	} else if (range) {
		phys_addr_t start = block->base;
		phys_addr_t end = block->base + block->count *
				  (phys_addr_t)BHPA_SIZE;
		phys_addr_t range_end = range->addr + range->size;

		if (range->addr > start)
			start = ALIGN(range->addr, BHPA_SIZE);
		if (range_end < end)
			end = ALIGN_DOWN(range_end, BHPA_SIZE);
		if (start >= end) {
			/* No overlap */
			last_page = first_page;
		} else {
			first_page = (start - block->base) / BHPA_SIZE;
			last_page = (end - block->base) / BHPA_SIZE;
		}
	}
	*p_first_page = first_page;
	*p_last_page = last_page;
}

/* bhpa_block_alloc_fast - Fast path allocator from BHPA block
 *
 * @block: BHPA block to allocate from
 * @pages: array for allocated pages
 * @count: number of entries in @pages
 * @allocated: number of actually allocated pages
 * @range: optional, restrict allocation to pages within this range
 * @order: allocation order
 */
static int bhpa_block_alloc_fast(struct bhpa_block *block,
				 uint64_t *pages,
				 unsigned int count,
				 unsigned int *allocated,
				 const struct brcmstb_range *range,
				 unsigned int order)
{
	unsigned int first_page = 0;
	unsigned int last_page = block->count;
	int rc = 0;
	unsigned long pfn_start_range;
	unsigned long pfn_end_range;
	phys_addr_t start;
	struct page *free_page = NULL;
	long prev_failed_bit = -1;
	unsigned int tries = 0, mt = block->mt;

	*allocated = 0;
	bhpa_block_update_range(block, range, &first_page, &last_page);
	B_LOG_DBG("bhpa_block_alloc_fast:%p free:%u/%u count:%u %u..%u",
		  block, block->free, block->count, count, first_page,
		  last_page);
	if (first_page == last_page)
		return rc;

	count = min(count, last_page - first_page);
	start = block->base + first_page * (phys_addr_t)BHPA_SIZE;
	pfn_start_range = (unsigned long)(start >> PAGE_SHIFT);
	pfn_end_range = (((last_page - first_page) * (phys_addr_t)BHPA_SIZE)
			 >> PAGE_SHIFT) + pfn_start_range;
	while (count) {
		phys_addr_t free_start;
		unsigned long free_bit;
		unsigned long pfn_start;
		unsigned long pfn_end;

		free_page = bhpa_get_free_page_in_range(pfn_start_range,
							pfn_end_range,
							block->mt,
							order);
		if (free_page == NULL) {
			B_LOG_TRACE("block_alloc_fast:%p:%u no free pages count:%u",
				    block, order, count);
			break;
		}
		free_start = page_to_phys(free_page);
		if (free_start < block->base)
			break;
		free_bit = (free_start - block->base) / BHPA_SIZE;
		B_LOG_TRACE("block_alloc_fast:%p:%u free:%u:(%lu)",
			    block, order, count, free_bit);
		if (WARN_ON(free_bit >= block->count) ||
		    WARN_ON(free_bit < first_page) ||
		    WARN_ON(free_bit >= last_page) ||
		    WARN_ON(test_bit(free_bit, block->allocated)))
			break;
		free_start = block->base + free_bit * (phys_addr_t)BHPA_SIZE;
		pfn_start = (unsigned long)(free_start >> PAGE_SHIFT);
		pfn_end = pfn_start + (BHPA_SIZE >> PAGE_SHIFT);
		rc = alloc_contig_range(pfn_start, pfn_end, mt, GFP_ATOMIC);
		if (rc == -EINTR)
			break;
		if (rc != 0) {
			B_LOG_DBG("block_alloc_fast:%p: failed : %u:(%lu,prev:%ld) %#llx (%lx..%lx) %#llx",
				  block, count, free_bit, prev_failed_bit,
				  (unsigned long long)free_start, pfn_start,
				  pfn_end,
				  (unsigned long long)((phys_addr_t)
						        pfn_start << PAGE_SHIFT));
			rc = 0;
			if (prev_failed_bit == free_bit)
				/*
				 *  we wouldn't get any different free pages,
				 *  and it can't be allocated, so bail out
				 */
				break;

			prev_failed_bit = free_bit;
			tries++;
			if (tries > 10)
				break;
			continue; /* keep on trying */
		}
		B_LOG_DBG("block_alloc_fast:%p:%u allocated: %u:(%lu) %#llx (%lx..%lx)",
			  block, order, count, free_bit,
			  (unsigned long long)free_start, pfn_start, pfn_end);
		if (test_and_set_bit(free_bit, block->busy)) {
			block->stats.busy--;
			block->free++;
		}

		if (!WARN_ON(!block->free))
			block->free--;

		block->stats.allocated++;
		block->stats.high_allocated = max(block->stats.high_allocated,
						  block->stats.allocated);
		WARN_ON((BHPA_ORDER - order) >=
			ARRAY_SIZE(block->stats.allocations.fast));
		block->stats.allocations.fast[BHPA_ORDER - order]++;
		set_bit(free_bit, block->allocated);
		pages[*allocated] = free_start;
		(*allocated)++;
		count--;
	}
	return rc;
}

/* bhpa_block_alloc - Slow path allocator from BHPA block
 *
 * @block: BHPA block to allocate from
 * @pages: array for allocated pages
 * @count: number of entries in @pages
 * @allocated: number of actually allocated pages
 * @range: optional, restrict allocation to pages within this range
 */
static int bhpa_block_alloc(struct bhpa_block *block,
			    uint64_t *pages,
			    unsigned int count,
			    unsigned int *allocated,
			    const struct brcmstb_range *range)
{
	unsigned int first_page = 0;
	unsigned int last_page = block->count, mt = block->mt;
	int rc = 0;

	*allocated = 0;
	bhpa_block_update_range(block, range, &first_page, &last_page);

	B_LOG_DBG("bhpa_block_alloc:%p free:%u/%u count:%u %u..%u",
		  block, block->free, block->count, count, first_page,
		  last_page);
	if (first_page == last_page)
		return rc;
	count = min(count, last_page - first_page);

	while (count > 0) {
		unsigned long pfn_start;
		unsigned long pfn_end;
		unsigned long free_bit;
		unsigned int max_pages;
		phys_addr_t start;
		unsigned int i;
		unsigned int free_count;

		free_bit = find_next_zero_bit(block->busy, BHPA_BLOCK_PAGES,
					      first_page);
		B_LOG_TRACE("block_alloc:%p count:%u %u..%u -> %lu", block,
			    count, first_page, last_page, free_bit);
		if (free_bit >= last_page)
			break;
		max_pages = min3(8U, count, last_page - (unsigned int)free_bit);
		/* try to extend range of pages */
		free_count = 1;
		while (free_count < max_pages) {
			if (!test_bit(free_bit + free_count, block->busy))
				free_count++;
			else
				break;
		}
		start = block->base + free_bit * (phys_addr_t)BHPA_SIZE;
		pfn_start = (unsigned long)(start >> PAGE_SHIFT);
		pfn_end = pfn_start + free_count * (BHPA_SIZE >> PAGE_SHIFT);
		B_LOG_TRACE("block_alloc:%p: %lu:%u (%lx..%lx) %#llx",
			    block, free_bit, free_count, pfn_start,
			    pfn_end, (unsigned long long)start);
		rc = alloc_contig_range(pfn_start, pfn_end, mt, GFP_KERNEL);
		if (rc != 0) {
			B_LOG_DBG("block_alloc:%p: %lu:%u (%lx..%lx) %#llx failed:%d",
				  block, free_bit, free_count, pfn_start,
				  pfn_end, (unsigned long long)start, rc);
		}
		if (rc == -EINTR)
			break;
		if (rc != 0 && free_count != 1) {
			/* if multi-page allocation failed, try single page */
			free_count = 1;
			pfn_end = pfn_start + free_count *
				  (BHPA_SIZE >> PAGE_SHIFT);
			B_LOG_TRACE("block_alloc:%p: %lu (%lx..%lx) %#llx",
				    block, free_bit, pfn_start, pfn_end,
				    (unsigned long long)start);
			rc = alloc_contig_range(pfn_start, pfn_end, mt,
						GFP_KERNEL);
			if (rc != 0) {
				B_LOG_DBG("block_alloc:%p: %lu (%lx..%lx) %#llx failed:%d",
					  block, free_bit, pfn_start, pfn_end,
					  (unsigned long long)start, rc);
			}
			if (rc == -EINTR)
				break;
		}
		for (i = 0; i < free_count; i++)
			set_bit(free_bit + i, block->busy);
		block->free -= free_count;
		first_page = free_bit + free_count;
		if (rc == 0) {
			B_LOG_DBG("block_alloc:%p: allocated: %u:%#llx(%lu) %#llx pages:%u",
				  block, count, (unsigned long long)start,
				  free_bit, (unsigned long long)start,
				  free_count);
			count -= free_count;
			block->stats.allocations.slow += free_count;
			block->stats.allocated += free_count;
			block->stats.high_allocated =
				max(block->stats.high_allocated,
				    block->stats.allocated);
			for (i = 0; i < free_count; i++) {
				set_bit(free_bit + i, block->allocated);
				pages[*allocated] = start + i * BHPA_SIZE;
				(*allocated)++;
			}
		} else {
			B_LOG_DBG("block_alloc:%p: can't be allocated: %u:%#llx(%lu)",
				  block, count, (unsigned long long)start,
				  free_bit);
			block->stats.busy += free_count;
			block->stats.high_busy = max(block->stats.high_busy,
						     block->stats.busy);
			rc = 0;
		}
	}
	return rc;
}

static unsigned int bhpa_block_clear_busy(struct bhpa_block *block)
{
	unsigned int prev_free = block->free;
	unsigned int i;

	B_LOG_TRACE(">block_clear_busy:%p free:%u", block, block->free);
	for (i = 0;;) {
		unsigned long free_bit;

		free_bit = find_next_zero_bit(block->allocated,
					      BHPA_BLOCK_PAGES, i);
		B_LOG_TRACE("block_clear_busy:%p free_bit:%u->%u", block, i,
			    (unsigned int)free_bit);
		if (free_bit >= block->count)
			break;
		if (test_and_clear_bit(free_bit, block->busy) &&
		    !WARN_ON(block->free >= block->count))
			block->free++;
		i = free_bit + 1;
	}
	B_LOG_TRACE("<block_clear_busy:%p free:%u", block, block->free);
	B_LOG_DBG("block_alloc:%p cleared:%u/%u pages free:%u(from %u)",
		  block, block->free - prev_free, block->stats.busy,
		  block->free, block->count);
	WARN_ON(block->stats.busy != block->free - prev_free);
	block->stats.busy = 0;

	return block->free - prev_free;
}

static void bhpa_block_free(struct bhpa_block *block,
			    phys_addr_t page)
{
	unsigned int page_no;
	unsigned long pfn_page;

	if (WARN_ON(page < block->base || page % BHPA_SIZE))
		return;

	page_no = (page - block->base) / BHPA_SIZE;
	B_LOG_DBG("bhpa_block_free:%p free:%u/%u page:%#llx page_no:%u",
		  block, block->free, block->count,
		  (unsigned long long)page, page_no);

	if (WARN_ON(page_no >= block->count ||
		    !test_bit(page_no, block->allocated) ||
		    !test_bit(page_no, block->busy) ||
		    block->free >= block->count))
		return;

	clear_bit(page_no, block->busy);
	clear_bit(page_no, block->allocated);
	block->free++;
	WARN_ON(block->stats.allocated <= 0);
	block->stats.allocated--;
	pfn_page = (unsigned long)(page >> PAGE_SHIFT);
	free_contig_range(pfn_page, BHPA_SIZE / PAGE_SIZE);
}

static __init void bhpa_memc_init(struct bhpa_memc *a)
{
	INIT_LIST_HEAD(&a->blocks);
}

static void bhpa_memc_free(struct bhpa_memc *a, const uint64_t *pages,
			   unsigned int count)
{
	struct bhpa_block *block;

	while (count > 0) {
		phys_addr_t page = *pages;
		bool found = false;

		list_for_each_entry(block, &a->blocks, node) {
			if (page >= block->base &&
			    page < block->base + (block->count * BHPA_SIZE)) {
				bhpa_block_free(block, page);
				found = true;
				break;
			}
		}
		B_LOG_TRACE("bhpa_memc_free:%p pages:%lx", a,
			    (unsigned long)page);
		WARN_ON(!found);
		pages++;
		count--;
	}
}

static void bhpa_memc_print(struct seq_file *seq, const struct bhpa_memc *a,
			    unsigned int memcIndex)
{
	const struct bhpa_block *block;

	list_for_each_entry(block, &a->blocks, node)
		bhpa_block_print(seq, block, memcIndex);
}


static int bhpa_memc_alloc(struct bhpa_memc *a, unsigned memcIndex, uint64_t *pages,
			   unsigned int count, unsigned int *allocated,
			   const struct brcmstb_range *range, gfp_t flags)
{
	struct bhpa_block *block;
	int rc = 0;
	unsigned int pass;
	unsigned int page_size;

	*allocated = 0;
	if (list_empty(&a->blocks))
		return -ENOMEM;

	for (page_size = BHPA_SIZE; page_size >= BHPA_SIZE / 8;
	     page_size = page_size / 2) {
		unsigned int order = get_order(page_size);

		list_for_each_entry(block, &a->blocks, node) {
			unsigned int block_allocated;

			rc = bhpa_block_alloc_fast(block, pages, count,
						   &block_allocated, range,
						   order);
			*allocated += block_allocated;
			pages += block_allocated;
			count -= block_allocated;
			if (rc != 0)
				goto done;
			if (count == 0)
				goto done;
		}
	}
	for (pass = 0; pass < 2; pass++) {
		list_for_each_entry(block, &a->blocks, node) {
			unsigned int block_allocated;

			rc = bhpa_block_alloc(block, pages, count,
					      &block_allocated, range);
			if (pass == 0 && count == block_allocated) {
				B_LOG_TRACE("bhpa_memc_alloc:%p pages:%u/%u pass:%u",
					    block, block_allocated, count,
					    pass);
			} else {
				B_LOG_DBG("bhpa_memc_alloc:%p pages:%u/%u pass:%u",
					  block, block_allocated, count, pass);
			}
			*allocated += block_allocated;
			pages += block_allocated;
			count -= block_allocated;
			if (rc != 0)
				break;
			if (count == 0)
				goto done;
		}
		if (pass == 0) { /* clear all busy, but not allocated pages */
				 /* and try again */
			unsigned int cleared = 0;

			list_for_each_entry(block, &a->blocks, node) {
				cleared += bhpa_block_clear_busy(block);
			}
			if (cleared == 0)
				break;
		}
	}
done:
	if (rc == 0) {
		if (count != 0 && range == NULL && !(flags & __GFP_NOWARN)) {
			pr_err("BHPA MEMC%u Out of memory\n", memcIndex);
			bhpa_memc_print(NULL, a, memcIndex);
			dump_stack();
		}
	} else {
		/* in case of error free all partially allocated memory */
		bhpa_memc_free(a, pages - *allocated, *allocated);
		*allocated = 0;
	}

	return rc;
}

void brcmstb_hpa_print(struct seq_file *seq)
{
	unsigned int i;

	mutex_lock(&bhpa_lock);
	for (i = 0; i < MAX_BRCMSTB_MEMC; i++)
		bhpa_memc_print(seq, &bhpa_allocator.memc[i], i);

	mutex_unlock(&bhpa_lock);
}

static int bhpa_debugfs_show(struct seq_file *seq, void *p)
{
	brcmstb_hpa_print(seq);
	return 0;
}

static int bhpa_debugfs_open(struct inode *inode,
	struct file *file)
{
	return single_open(file, bhpa_debugfs_show, inode->i_private);
}

static const struct file_operations bhpa_debugfs_fops = {
	.open       = bhpa_debugfs_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release    = single_release,
};

static __init void bhpa_debugfs_init(struct bhpa_allocator *a)
{
	a->debugfs = debugfs_create_file("bhpa", 0444, NULL, a,
					 &bhpa_debugfs_fops);
	if (IS_ERR_OR_NULL(a->debugfs))
		a->debugfs = NULL;
}

static __init void bhpa_allocator_init(struct bhpa_allocator *a)
{
	unsigned int i;

	for (i = 0; i < MAX_BRCMSTB_MEMC; i++)
		bhpa_memc_init(&a->memc[i]);
}

static void __exit bhpa_allocator_exit(struct bhpa_allocator *a)
{
	struct bhpa_block *b, *tmp;
	unsigned long pfn;
	unsigned int i, idx;

	for (i = 0; i < MAX_BRCMSTB_MEMC; i++) {
		list_for_each_entry_safe(b, tmp, &a->memc[i].blocks, node) {
			pfn = PFN_DOWN(b->base);
			for (idx = 0; idx < b->count; idx++) {
				if (test_bit(idx, b->allocated)) {
					pr_info("free huge page: 0x%lx\n", pfn);
					free_contig_range(pfn, 1 << BHPA_ORDER);
				}
				pfn += (1 << BHPA_ORDER);
			}
			list_del(&b->node);
			kfree(b);
		}
	}

	if (!IS_ERR_OR_NULL(a->debugfs))
		debugfs_remove(a->debugfs);
}

static __init unsigned int bhpa_trim_memory(phys_addr_t *base, phys_addr_t end)
{
	phys_addr_t _base = ALIGN(*base, BHPA_SIZE);

	*base = _base;
	if (_base >= end)
		return 0;

	return (end - _base)/BHPA_SIZE;
}

static __init int bhpa_memc_add_memory(struct bhpa_memc *a, phys_addr_t base,
				       phys_addr_t end, unsigned mt)
{
	unsigned int total_pages = bhpa_trim_memory(&base, end);
	struct list_head *head = a->blocks.next;

	while (total_pages) {
		struct bhpa_block *block = kmalloc(sizeof(*block), GFP_KERNEL);
		unsigned int pages;

		if (total_pages > BHPA_BLOCK_PAGES)
			pages = BHPA_BLOCK_PAGES;
		else
			pages = total_pages;

		if (block) {
			bhpa_block_init(block, base, pages, mt);
			B_LOG_DBG("Adding list... %p %p page:%u", &a->blocks,
				  &block->node, pages);
			list_add(&block->node, &a->blocks);
		} else {
			struct bhpa_block *tmp, *p;

			list_for_each_entry_safe(p, tmp, &a->blocks, node) {
				if (&p->node == head)
					break;
				list_del(&p->node);
				kfree(p);
			}
			return -ENOMEM;
		}

		total_pages -= pages;
		base += BHPA_SIZE * BHPA_BLOCK_PAGES;
	}
	return 0;
}

static __init int bhpa_add_memory(struct bhpa_region *p)
{
	phys_addr_t end = p->addr + p->size;
	int rc;

	if (WARN_ON(p->memc < 0 || p->memc >= MAX_BRCMSTB_MEMC))
		return -1;

	mutex_lock(&bhpa_lock);
	rc = bhpa_memc_add_memory(&bhpa_allocator.memc[p->memc], p->addr, end,
				  p->mt);
	mutex_unlock(&bhpa_lock);

	return rc;
}

/*
 * brcmstb_hpa_alloc() - Allocate 2MB pages from ZONE_MOVABLE
 *
 * @memcIndex: memory controller
 * @pages: array for allocated pages
 * @count: number of entries in pages array above
 * @alloced: number of actually allocated pages
 * @range: optional, restrict allocation to pages within this range
 *
 * Return: 0 on success, negative on failure.
 */
int brcmstb_hpa_alloc(unsigned int memcIndex, uint64_t *pages,
		      unsigned int count, unsigned int *alloced,
		      const struct brcmstb_range *range, gfp_t flags)
{
	int rc;

	if (memcIndex >= MAX_BRCMSTB_MEMC || !pages || !count || !alloced)
		return -EINVAL;

	*alloced = 0;

	rc = mutex_lock_interruptible(&bhpa_lock);
	if (rc != 0)
		return rc;

	rc = bhpa_memc_alloc(&bhpa_allocator.memc[memcIndex], memcIndex, pages, count,
			     alloced, range, flags);

	mutex_unlock(&bhpa_lock);
	return rc;
}

/*
 * brcmstb_hpa_free() - Release 2MB pages allocated by brcmstb_hpa_alloc().
 *
 * @memcIndex: memory controller
 * @pages: array for allocated pages
 * @count: number of entries in pages array above
 *
 */
void brcmstb_hpa_free(unsigned int memcIndex, const uint64_t *pages,
		      unsigned int count)
{
	if (WARN_ON(memcIndex >= MAX_BRCMSTB_MEMC || !pages || !count))
		return;

	mutex_lock(&bhpa_lock);
	bhpa_memc_free(&bhpa_allocator.memc[memcIndex], pages, count);
	mutex_unlock(&bhpa_lock);
}

/*
 * Returns index if the supplied range falls entirely within a bhpa region
 */
int brcmstb_hugepage_find_region(phys_addr_t addr, phys_addr_t size)
{
	int i;

	for (i = 0; i < n_bhpa_regions; i++) {
		if (addr < bhpa_regions[i].addr)
			return -ENOENT;

		if (addr + size <=
		    bhpa_regions[i].addr + bhpa_regions[i].size)
			return i;
	}
	return -ENOENT;
}
EXPORT_SYMBOL(brcmstb_hugepage_find_region);

/*
 * Finds the IDX'th bhpa region, and fills in addr/size if it exists.
 * Returns 0 on success, <0 on failure.
 * Can pass in NULL for addr and/or size if you only care about return value.
 */
int brcmstb_hugepage_region_info(int idx, phys_addr_t *addr, phys_addr_t *size)
{
	if (idx >= n_bhpa_regions)
		return -ENOENT;

	if (addr)
		*addr = bhpa_regions[idx].addr;
	if (size)
		*size = bhpa_regions[idx].size;

	return 0;
}
EXPORT_SYMBOL(brcmstb_hugepage_region_info);

void brcmstb_hugepage_print(struct seq_file *seq)
{
	brcmstb_hpa_print(seq);
}
EXPORT_SYMBOL(brcmstb_hugepage_print);

int brcmstb_hugepage_alloc(unsigned int memcIndex, uint64_t *pages,
			   unsigned int count, unsigned int *allocated,
			   const struct brcmstb_range *range)
{
	return brcmstb_hpa_alloc(memcIndex, pages, count, allocated, range, 0);
}
EXPORT_SYMBOL(brcmstb_hugepage_alloc);

int __brcmstb_hugepage_alloc(unsigned int memcIndex, uint64_t *pages,
			     unsigned int count, unsigned int *allocated,
			     const struct brcmstb_range *range, gfp_t flags)
{
	return brcmstb_hpa_alloc(memcIndex, pages, count, allocated, range, flags);
}
EXPORT_SYMBOL(__brcmstb_hugepage_alloc);

void brcmstb_hugepage_free(unsigned int memcIndex, const uint64_t *pages,
			   unsigned int count)
{
	return brcmstb_hpa_free(memcIndex, pages, count);
}
EXPORT_SYMBOL(brcmstb_hugepage_free);

int __init bhpa_init(void)
{
	int rc, i, memc, added = 0;
	struct device_node *dn;
	struct bhpa_region *p;
	struct reserved_mem *rmem;

	B_LOG_DBG("Init");
	bhpa_allocator_init(&bhpa_allocator);

	for_each_compatible_node(dn, NULL, "brcm,bhpa") {
		rmem = of_reserved_mem_lookup(dn);
		if (!rmem)
			continue;

		if (n_bhpa_regions >= MAX_BHPA_REGIONS) {
			B_LOG_WRN("Only %d BHPA regions aupported",
				  MAX_BHPA_REGIONS);
			break;
		}
		memc = bhpa_memory_phys_addr_to_memc(rmem->base);
		if (bhpa_memory_phys_addr_to_memc(rmem->base + rmem->size)
		    != memc) {
			B_LOG_WRN("BHPA regions must be on a single memc");
			continue;
		}
		if (is_migrate_cma_page(pfn_to_page(PFN_DOWN(rmem->base))))
			__bhpa_setup(rmem->base, rmem->size, memc, MIGRATE_CMA);
		else
			B_LOG_WRN("brcm,bhpa: non-CMA reserved memory ignored");
	}

	bhpa_parameter_parse_ranges(ranges);

	if (!n_bhpa_regions)
		return -ENOMEM;

	B_LOG_DBG("Adding memory");
	for (i = 0; i < n_bhpa_regions; i++) {
		p = &bhpa_regions[i];
		rc = bhpa_add_memory(&bhpa_regions[i]);
		B_LOG_DBG("Adding memory [0x%pa] size = 0x%pa -> %d", &p->addr, &p->size, rc);
		if (!rc)
			added++;
	}
	if (!added)
		return -ENOMEM;

	bhpa_debugfs_init(&bhpa_allocator);

	return 0;
}
module_init(bhpa_init);

void __exit bhpa_exit(void)
{
	bhpa_allocator_exit(&bhpa_allocator);
}
module_exit(bhpa_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Brcmstb Huge Page Allocator");
MODULE_AUTHOR("Broadcom");
