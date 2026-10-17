/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _LINUX_LMB_H
#define _LINUX_LMB_H
#ifdef __KERNEL__

#include <asm/types.h>
#include <asm/u-boot.h>

/*
 * Logical memory blocks.
 *
 * Copyright (C) 2001 Peter Bergner, IBM Corp.
 */

/**
 * enum lmb_flags - definition of memory region attributes
 * @LMB_NONE: no special request
 * @LMB_NOMAP: don't add to mmu configuration
 */
enum lmb_flags {
	LMB_NONE		= 0x0,
	LMB_NOMAP		= 0x4,
};

/**
 * struct lmb_property - Description of one region.
 *
 * @base:	Base address of the region.
 * @size:	Size of the region
 * @flags:	memory region attributes
 */
struct lmb_property {
	phys_addr_t base;
	phys_size_t size;
	enum lmb_flags flags;
};

/*
 * For regions size management, see LMB configuration in KConfig
 * all the #if test are done with CONFIG_LMB_USE_MAX_REGIONS (boolean)
 *
 * case 1. CONFIG_LMB_USE_MAX_REGIONS is defined (legacy mode)
 *         => CONFIG_LMB_MAX_REGIONS is used to configure the region size,
 *         directly in the array lmb_region.region[], with the same
 *         configuration for memory and reserved regions.
 *
 * case 2. CONFIG_LMB_USE_MAX_REGIONS is not defined, the size of each
 *         region is configurated *independently* with
 *         => CONFIG_LMB_MEMORY_REGIONS: struct lmb.memory_regions
 *         => CONFIG_LMB_RESERVED_REGIONS: struct lmb.reserved_regions
 *         lmb_region.region is only a pointer to the correct buffer,
 *         initialized in lmb_init(). This configuration is useful to manage
 *         more reserved memory regions with CONFIG_LMB_RESERVED_REGIONS.
 */

/**
 * struct lmb_region - Description of a set of region.
 *
 * @cnt: Number of regions.
 * @max: Size of the region array, max value of cnt.
 * @region: Array of the region properties
 */
struct lmb_region {
	unsigned long cnt;
	unsigned long max;
#if IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
	struct lmb_property region[CONFIG_LMB_MAX_REGIONS];
#else
	struct lmb_property *region;
#endif
};

/**
 * struct lmb - Logical memory block handle.
 *
 * Clients provide storage for Logical memory block (lmb) handles.
 * The content of the structure is managed by the lmb library.
 * A lmb struct is  initialized by lmb_init() functions.
 * The lmb struct is passed to all other lmb APIs.
 *
 * @memory: Description of memory regions.
 * @reserved: Description of reserved regions.
 * @memory_regions: Array of the memory regions (statically allocated)
 * @reserved_regions: Array of the reserved regions (statically allocated)
 */
struct lmb {
	struct lmb_region memory;
	struct lmb_region reserved;
#if !IS_ENABLED(CONFIG_LMB_USE_MAX_REGIONS)
	struct lmb_property memory_regions[CONFIG_LMB_MEMORY_REGIONS];
	struct lmb_property reserved_regions[CONFIG_LMB_RESERVED_REGIONS];
#endif
};

void lmb_init(struct lmb *lmb);
void lmb_init_and_reserve(struct lmb *lmb, struct bd_info *bd, void *fdt_blob);
void lmb_init_and_reserve_range(struct lmb *lmb, phys_addr_t base,
				phys_size_t size, void *fdt_blob);
long lmb_add(struct lmb *lmb, phys_addr_t base, phys_size_t size);
long lmb_reserve(struct lmb *lmb, phys_addr_t base, phys_size_t size);
/**
 * lmb_reserve_flags - Reserve one region with a specific flags bitfield.
 *
 * @lmb:	the logical memory block struct
 * @base:	base address of the memory region
 * @size:	size of the memory region
 * @flags:	flags for the memory region
 * Return:	0 if OK, > 0 for coalesced region or a negative error code.
 */
long lmb_reserve_flags(struct lmb *lmb, phys_addr_t base,
		       phys_size_t size, enum lmb_flags flags);
phys_addr_t lmb_alloc(struct lmb *lmb, phys_size_t size, ulong align);
phys_addr_t lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align,
			   phys_addr_t max_addr);
phys_addr_t __lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align,
			     phys_addr_t max_addr);
phys_addr_t lmb_alloc_addr(struct lmb *lmb, phys_addr_t base, phys_size_t size);
phys_size_t lmb_get_free_size(struct lmb *lmb, phys_addr_t addr);

/**
 * lmb_is_reserved() - test if address is in reserved region
 *
 * The function checks if a reserved region comprising @addr exists.
 *
 * @lmb:	the logical memory block struct
 * @addr:	address to be tested
 * Return:	1 if reservation exists, 0 otherwise
 */
int lmb_is_reserved(struct lmb *lmb, phys_addr_t addr);

/**
 * lmb_is_reserved_flags() - test if address is in reserved region with flag bits set
 *
 * The function checks if a reserved region comprising @addr exists which has
 * all flag bits set which are set in @flags.
 *
 * @lmb:	the logical memory block struct
 * @addr:	address to be tested
 * @flags:	bitmap with bits to be tested
 * Return:	1 if matching reservation exists, 0 otherwise
 */
int lmb_is_reserved_flags(struct lmb *lmb, phys_addr_t addr, int flags);

long lmb_free(struct lmb *lmb, phys_addr_t base, phys_size_t size);

void lmb_dump_all(struct lmb *lmb);
void lmb_dump_all_force(struct lmb *lmb);

void board_lmb_reserve(struct lmb *lmb);
void arch_lmb_reserve(struct lmb *lmb);
void arch_lmb_reserve_generic(struct lmb *lmb, ulong sp, ulong end, ulong align);

#endif /* __KERNEL__ */

#endif /* _LINUX_LMB_H */
