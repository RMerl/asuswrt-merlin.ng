/*
 * Copyright © 2022 Broadcom
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

#ifndef _BRCMSTB_MEMORY_API_H
#define _BRCMSTB_MEMORY_API_H

#include <linux/gfp.h>

/*
 * Memory API that must be supplied for Broadcom STB middleware.
 *
 * **********************
 * READ ME BEFORE EDITING
 * **********************
 *
 * If you update this file, make sure to bump BRCMSTB_H_VERSION
 * in brcmstb.h if there is an API change!
 */

#define MAX_BRCMSTB_RANGE           16
#define MAX_BRCMSTB_MEMC            3
#define MAX_BRCMSTB_RESERVED_RANGE  16
#define MAX_BRCMSTB_RESERVED_NAME   8

struct brcmstb_range {
	u64 addr;
	u64 size;  /* 0 means no region */
};

struct brcmstb_named_range {
	char name[MAX_BRCMSTB_RESERVED_NAME];
};

struct brcmstb_reserved_memory {
	struct brcmstb_range range[MAX_BRCMSTB_RESERVED_RANGE];
	struct brcmstb_named_range range_name[MAX_BRCMSTB_RESERVED_RANGE];
	int count;
};

struct brcmstb_memory {
	struct {
		struct brcmstb_range range[MAX_BRCMSTB_RANGE];
		int count;
	} memc[MAX_BRCMSTB_MEMC];
	/* fixed map space */
	struct {
		struct brcmstb_range range[MAX_BRCMSTB_RANGE];
		int count;
	} lowmem;
	/* bmem carveout regions */
	struct {
		struct brcmstb_range range[MAX_BRCMSTB_RANGE];
		int count;
	} bmem;
	/* CMA carveout regions */
	struct {
		struct brcmstb_range range[MAX_BRCMSTB_RANGE];
		int count;
	} cma;
	/* regions that nexus cannot recommend for bmem or CMA */
	struct brcmstb_reserved_memory reserved;
	/* bhpa regions */
	struct {
		struct brcmstb_range range[MAX_BRCMSTB_RANGE];
		int count;
	} bhpa;
};

#define BRCMSTB_MEM_SUPPORTS_UNALIGNED_KVA_MAP
#if IS_ENABLED(CONFIG_BRCMSTB_MEMORY_API)
int brcmstb_memory_phys_addr_to_memc(phys_addr_t pa);
#endif
void *brcmstb_memory_kva_map(struct page *page, int num_pges, pgprot_t pgprot);
void *brcmstb_memory_kva_map_phys(phys_addr_t phys, size_t size, bool cached);
int brcmstb_memory_kva_unmap(const void *kva);

/* Below functions are for calling during initialization and may need stubs */
void __init brcmstb_memory_init(void);
void __init brcmstb_memory_default_reserve(int (*setup)(phys_addr_t start,
							phys_addr_t size));

#if IS_ENABLED(CONFIG_BRCMSTB_MEMORY_API)
void __init brcmstb_memory_reserve(void);
int brcmstb_memory_get(struct brcmstb_memory *mem);
#else
static inline void brcmstb_memory_reserve(void) {};
static inline int brcmstb_memory_get(struct brcmstb_memory *mem)
{
	return -ENOSYS;
}
#endif

#define for_each_range(bm, __memc, range)				\
	for (range = bm.memc[__memc].range;				\
	     range < (bm.memc[__memc].range + bm.memc[__memc].count);	\
	     range++)

#define for_each_range_of_memc(bm, memc, range)				\
	for (memc = 0; memc < MAX_BRCMSTB_MEMC; memc++)			\
		for_each_range(bm, memc, range)				\

/* The following relate to the default reservation scheme */

enum brcmstb_reserve_type {
	BRCMSTB_RESERVE_BMEM,
	BRCMSTB_RESERVE_CMA,
	BRCMSTB_RESERVE_BHPA,
};

/* Determines what type of memory reservation will be used w/o CLI params */
extern enum brcmstb_reserve_type brcmstb_default_reserve;
/* Should be set to true by any CLI option that overrides default reserve */
extern bool brcmstb_memory_override_defaults;
/* May be set to true by any CLI option to convert bmem ranges to bhpa */
extern bool brcmstb_bmem_is_bhpa;


#if IS_ENABLED(CONFIG_BRCMSTB_HUGEPAGES)
int brcmstb_hugepage_find_region(phys_addr_t addr, phys_addr_t size);
int brcmstb_hugepage_region_info(int idx, phys_addr_t *addr, phys_addr_t *size);
int brcmstb_hugepage_alloc(unsigned int memcIndex, uint64_t *pages,
			   unsigned int count, unsigned int *allocated,
			   const struct brcmstb_range *range);
int __brcmstb_hugepage_alloc(unsigned int memcIndex, uint64_t *pages,
			     unsigned int count, unsigned int *allocated,
			     const struct brcmstb_range *range, gfp_t flags);
void brcmstb_hugepage_free(unsigned int memcIndex, const uint64_t *pages,
			   unsigned int count);
#else
static inline
int brcmstb_hugepage_find_region(phys_addr_t addr, phys_addr_t size)
{
	return -ENOSYS;
}

static inline
int brcmstb_hugepage_region_info(int idx, phys_addr_t *addr, phys_addr_t *size)
{
	return -ENOSYS;
}

static inline
int brcmstb_hugepage_alloc(unsigned int memcIndex, uint64_t *pages,
			   unsigned int count, unsigned int *allocated,
			   const struct brcmstb_range *range)
{
	return -ENOSYS;
}

static inline
int __brcmstb_hugepage_alloc(unsigned int memcIndex, uint64_t *pages,
			     unsigned int count, unsigned int *allocated,
			     const struct brcmstb_range *range, gfp_t flags)
{
	return -ENOSYS;
}

static inline
void brcmstb_hugepage_free(unsigned int memcIndex, const uint64_t *pages,
			   unsigned int count)
{
}
#endif /* CONFIG_BRCMSTB_HUGEPAGES */

#if IS_BUILTIN(CONFIG_BRCMSTB_MEMORY_API)
void __iomem *brcmstb_ioremap(phys_addr_t phys_addr, size_t size);
#else
#define brcmstb_ioremap	ioremap
#endif
#define brcmstb_iounmap	iounmap

#endif  /* _BRCMSTB_MEMORY_API_H */
