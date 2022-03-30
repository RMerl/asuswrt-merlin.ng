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

#define MAX_LMB_REGIONS 8

struct lmb_property {
	phys_addr_t base;
	phys_size_t size;
};

struct lmb_region {
	unsigned long cnt;
	phys_size_t size;
	struct lmb_property region[MAX_LMB_REGIONS+1];
};

struct lmb {
	struct lmb_region memory;
	struct lmb_region reserved;
};

extern void lmb_init(struct lmb *lmb);
extern void lmb_init_and_reserve(struct lmb *lmb, bd_t *bd, void *fdt_blob);
extern void lmb_init_and_reserve_range(struct lmb *lmb, phys_addr_t base,
				       phys_size_t size, void *fdt_blob);
extern long lmb_add(struct lmb *lmb, phys_addr_t base, phys_size_t size);
extern long lmb_reserve(struct lmb *lmb, phys_addr_t base, phys_size_t size);
extern phys_addr_t lmb_alloc(struct lmb *lmb, phys_size_t size, ulong align);
extern phys_addr_t lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align,
			    phys_addr_t max_addr);
extern phys_addr_t __lmb_alloc_base(struct lmb *lmb, phys_size_t size, ulong align,
			      phys_addr_t max_addr);
extern phys_addr_t lmb_alloc_addr(struct lmb *lmb, phys_addr_t base,
				  phys_size_t size);
extern phys_size_t lmb_get_free_size(struct lmb *lmb, phys_addr_t addr);
extern int lmb_is_reserved(struct lmb *lmb, phys_addr_t addr);
extern long lmb_free(struct lmb *lmb, phys_addr_t base, phys_size_t size);

extern void lmb_dump_all(struct lmb *lmb);

static inline phys_size_t
lmb_size_bytes(struct lmb_region *type, unsigned long region_nr)
{
	return type->region[region_nr].size;
}

void board_lmb_reserve(struct lmb *lmb);
void arch_lmb_reserve(struct lmb *lmb);

#endif /* __KERNEL__ */

#endif /* _LINUX_LMB_H */
