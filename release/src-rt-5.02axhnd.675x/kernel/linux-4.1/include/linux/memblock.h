#ifndef _LINUX_MEMBLOCK_H
#define _LINUX_MEMBLOCK_H
#ifdef __KERNEL__

#ifdef CONFIG_HAVE_MEMBLOCK
/*
 * Logical memory blocks.
 *
 * Copyright (C) 2001 Peter Bergner, IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/mm.h>

#define INIT_MEMBLOCK_REGIONS	128
#define INIT_PHYSMEM_REGIONS	4

/* Definition of memblock flags. */
#define MEMBLOCK_HOTPLUG	0x1	/* hotpluggable region */

struct memblock_region {
	phys_addr_t base;
	phys_addr_t size;
	unsigned long flags;
#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
	int nid;
#endif
};

struct memblock_type {
	unsigned long cnt;	/* number of regions */
	unsigned long max;	/* size of the allocated array */
	phys_addr_t total_size;	/* size of all regions */
	struct memblock_region *regions;
};

struct memblock {
	bool bottom_up;  /* is bottom up direction? */
	phys_addr_t current_limit;
	struct memblock_type memory;
	struct memblock_type reserved;
#ifdef CONFIG_HAVE_MEMBLOCK_PHYS_MAP
	struct memblock_type physmem;
#endif
};

extern struct memblock memblock;
extern int memblock_debug;
#ifdef CONFIG_MOVABLE_NODE
/* If movable_node boot option specified */
extern bool movable_node_enabled;
#endif /* CONFIG_MOVABLE_NODE */

#define memblock_dbg(fmt, ...) \
	if (memblock_debug) printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)

phys_addr_t memblock_find_in_range_node(phys_addr_t size, phys_addr_t align,
					    phys_addr_t start, phys_addr_t end,
					    int nid);
phys_addr_t memblock_find_in_range(phys_addr_t start, phys_addr_t end,
				   phys_addr_t size, phys_addr_t align);
phys_addr_t get_allocated_memblock_reserved_regions_info(phys_addr_t *addr);
phys_addr_t get_allocated_memblock_memory_regions_info(phys_addr_t *addr);
void memblock_allow_resize(void);
int memblock_add_node(phys_addr_t base, phys_addr_t size, int nid);
int memblock_add(phys_addr_t base, phys_addr_t size);
int memblock_remove(phys_addr_t base, phys_addr_t size);
int memblock_free(phys_addr_t base, phys_addr_t size);
int memblock_reserve(phys_addr_t base, phys_addr_t size);
void memblock_trim_memory(phys_addr_t align);
int memblock_mark_hotplug(phys_addr_t base, phys_addr_t size);
int memblock_clear_hotplug(phys_addr_t base, phys_addr_t size);

/* Low level functions */
int memblock_add_range(struct memblock_type *type,
		       phys_addr_t base, phys_addr_t size,
		       int nid, unsigned long flags);

int memblock_remove_range(struct memblock_type *type,
			  phys_addr_t base,
			  phys_addr_t size);

void __next_mem_range(u64 *idx, int nid, struct memblock_type *type_a,
		      struct memblock_type *type_b, phys_addr_t *out_start,
		      phys_addr_t *out_end, int *out_nid);

void __next_mem_range_rev(u64 *idx, int nid, struct memblock_type *type_a,
			  struct memblock_type *type_b, phys_addr_t *out_start,
			  phys_addr_t *out_end, int *out_nid);

/**
 * for_each_mem_range - iterate through memblock areas from type_a and not
 * included in type_b. Or just type_a if type_b is NULL.
 * @i: u64 used as loop variable
 * @type_a: ptr to memblock_type to iterate
 * @type_b: ptr to memblock_type which excludes from the iteration
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 */
#define for_each_mem_range(i, type_a, type_b, nid,			\
			   p_start, p_end, p_nid)			\
	for (i = 0, __next_mem_range(&i, nid, type_a, type_b,		\
				     p_start, p_end, p_nid);		\
	     i != (u64)ULLONG_MAX;					\
	     __next_mem_range(&i, nid, type_a, type_b,			\
			      p_start, p_end, p_nid))

/**
 * for_each_mem_range_rev - reverse iterate through memblock areas from
 * type_a and not included in type_b. Or just type_a if type_b is NULL.
 * @i: u64 used as loop variable
 * @type_a: ptr to memblock_type to iterate
 * @type_b: ptr to memblock_type which excludes from the iteration
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 */
#define for_each_mem_range_rev(i, type_a, type_b, nid,			\
			       p_start, p_end, p_nid)			\
	for (i = (u64)ULLONG_MAX,					\
		     __next_mem_range_rev(&i, nid, type_a, type_b,	\
					 p_start, p_end, p_nid);	\
	     i != (u64)ULLONG_MAX;					\
	     __next_mem_range_rev(&i, nid, type_a, type_b,		\
				  p_start, p_end, p_nid))

#ifdef CONFIG_MOVABLE_NODE
static inline bool memblock_is_hotpluggable(struct memblock_region *m)
{
	return m->flags & MEMBLOCK_HOTPLUG;
}

static inline bool movable_node_is_enabled(void)
{
	return movable_node_enabled;
}
#else
static inline bool memblock_is_hotpluggable(struct memblock_region *m)
{
	return false;
}
static inline bool movable_node_is_enabled(void)
{
	return false;
}
#endif

#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
int memblock_search_pfn_nid(unsigned long pfn, unsigned long *start_pfn,
			    unsigned long  *end_pfn);
void __next_mem_pfn_range(int *idx, int nid, unsigned long *out_start_pfn,
			  unsigned long *out_end_pfn, int *out_nid);

/**
 * for_each_mem_pfn_range - early memory pfn range iterator
 * @i: an integer used as loop variable
 * @nid: node selector, %MAX_NUMNODES for all nodes
 * @p_start: ptr to ulong for start pfn of the range, can be %NULL
 * @p_end: ptr to ulong for end pfn of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over configured memory ranges.
 */
#define for_each_mem_pfn_range(i, nid, p_start, p_end, p_nid)		\
	for (i = -1, __next_mem_pfn_range(&i, nid, p_start, p_end, p_nid); \
	     i >= 0; __next_mem_pfn_range(&i, nid, p_start, p_end, p_nid))
#endif /* CONFIG_HAVE_MEMBLOCK_NODE_MAP */

/**
 * for_each_free_mem_range - iterate through free memblock areas
 * @i: u64 used as loop variable
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over free (memory && !reserved) areas of memblock.  Available as
 * soon as memblock is initialized.
 */
#define for_each_free_mem_range(i, nid, p_start, p_end, p_nid)		\
	for_each_mem_range(i, &memblock.memory, &memblock.reserved,	\
			   nid, p_start, p_end, p_nid)

/**
 * for_each_free_mem_range_reverse - rev-iterate through free memblock areas
 * @i: u64 used as loop variable
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over free (memory && !reserved) areas of memblock in reverse
 * order.  Available as soon as memblock is initialized.
 */
#define for_each_free_mem_range_reverse(i, nid, p_start, p_end, p_nid)	\
	for_each_mem_range_rev(i, &memblock.memory, &memblock.reserved,	\
			       nid, p_start, p_end, p_nid)

static inline void memblock_set_region_flags(struct memblock_region *r,
					     unsigned long flags)
{
	r->flags |= flags;
}

static inline void memblock_clear_region_flags(struct memblock_region *r,
					       unsigned long flags)
{
	r->flags &= ~flags;
}

#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
int memblock_set_node(phys_addr_t base, phys_addr_t size,
		      struct memblock_type *type, int nid);

static inline void memblock_set_region_node(struct memblock_region *r, int nid)
{
	r->nid = nid;
}

static inline int memblock_get_region_node(const struct memblock_region *r)
{
	return r->nid;
}
#else
static inline void memblock_set_region_node(struct memblock_region *r, int nid)
{
}

static inline int memblock_get_region_node(const struct memblock_region *r)
{
	return 0;
}
#endif /* CONFIG_HAVE_MEMBLOCK_NODE_MAP */

phys_addr_t memblock_alloc_nid(phys_addr_t size, phys_addr_t align, int nid);
phys_addr_t memblock_alloc_try_nid(phys_addr_t size, phys_addr_t align, int nid);

phys_addr_t memblock_alloc(phys_addr_t size, phys_addr_t align);

#ifdef CONFIG_MOVABLE_NODE
/*
 * Set the allocation direction to bottom-up or top-down.
 */
static inline void __init memblock_set_bottom_up(bool enable)
{
	memblock.bottom_up = enable;
}

/*
 * Check if the allocation direction is bottom-up or not.
 * if this is true, that said, memblock will allocate memory
 * in bottom-up direction.
 */
static inline bool memblock_bottom_up(void)
{
	return memblock.bottom_up;
}
#else
static inline void __init memblock_set_bottom_up(bool enable) {}
static inline bool memblock_bottom_up(void) { return false; }
#endif

/* Flags for memblock_alloc_base() amd __memblock_alloc_base() */
#define MEMBLOCK_ALLOC_ANYWHERE	(~(phys_addr_t)0)
#define MEMBLOCK_ALLOC_ACCESSIBLE	0

phys_addr_t __init memblock_alloc_range(phys_addr_t size, phys_addr_t align,
					phys_addr_t start, phys_addr_t end);
phys_addr_t memblock_alloc_base(phys_addr_t size, phys_addr_t align,
				phys_addr_t max_addr);
phys_addr_t __memblock_alloc_base(phys_addr_t size, phys_addr_t align,
				  phys_addr_t max_addr);
phys_addr_t memblock_phys_mem_size(void);
phys_addr_t memblock_mem_size(unsigned long limit_pfn);
phys_addr_t memblock_start_of_DRAM(void);
phys_addr_t memblock_end_of_DRAM(void);
void memblock_enforce_memory_limit(phys_addr_t memory_limit);
int memblock_is_memory(phys_addr_t addr);
int memblock_is_region_memory(phys_addr_t base, phys_addr_t size);
int memblock_is_reserved(phys_addr_t addr);
int memblock_is_region_reserved(phys_addr_t base, phys_addr_t size);

extern void __memblock_dump_all(void);

static inline void memblock_dump_all(void)
{
	if (memblock_debug)
		__memblock_dump_all();
}

/**
 * memblock_set_current_limit - Set the current allocation limit to allow
 *                         limiting allocations to what is currently
 *                         accessible during boot
 * @limit: New limit value (physical address)
 */
void memblock_set_current_limit(phys_addr_t limit);


phys_addr_t memblock_get_current_limit(void);

/*
 * pfn conversion functions
 *
 * While the memory MEMBLOCKs should always be page aligned, the reserved
 * MEMBLOCKs may not be. This accessor attempt to provide a very clear
 * idea of what they return for such non aligned MEMBLOCKs.
 */

/**
 * memblock_region_memory_base_pfn - Return the lowest pfn intersecting with the memory region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_memory_base_pfn(const struct memblock_region *reg)
{
	return PFN_UP(reg->base);
}

/**
 * memblock_region_memory_end_pfn - Return the end_pfn this region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_memory_end_pfn(const struct memblock_region *reg)
{
	return PFN_DOWN(reg->base + reg->size);
}

/**
 * memblock_region_reserved_base_pfn - Return the lowest pfn intersecting with the reserved region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_reserved_base_pfn(const struct memblock_region *reg)
{
	return PFN_DOWN(reg->base);
}

/**
 * memblock_region_reserved_end_pfn - Return the end_pfn this region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_reserved_end_pfn(const struct memblock_region *reg)
{
	return PFN_UP(reg->base + reg->size);
}

#define for_each_memblock(memblock_type, region)					\
	for (region = memblock.memblock_type.regions;				\
	     region < (memblock.memblock_type.regions + memblock.memblock_type.cnt);	\
	     region++)


#ifdef CONFIG_ARCH_DISCARD_MEMBLOCK
#define __init_memblock __meminit
#define __initdata_memblock __meminitdata
#else
#define __init_memblock
#define __initdata_memblock
#endif

#ifdef CONFIG_MEMTEST
extern void early_memtest(phys_addr_t start, phys_addr_t end);
#else
static inline void early_memtest(phys_addr_t start, phys_addr_t end)
{
}
#endif

#else
static inline phys_addr_t memblock_alloc(phys_addr_t size, phys_addr_t align)
{
	return 0;
}

#endif /* CONFIG_HAVE_MEMBLOCK */

#endif /* __KERNEL__ */

#endif /* _LINUX_MEMBLOCK_H */
