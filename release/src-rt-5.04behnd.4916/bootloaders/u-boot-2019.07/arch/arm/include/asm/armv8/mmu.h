/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#ifndef _ASM_ARMV8_MMU_H_
#define _ASM_ARMV8_MMU_H_

#include <linux/const.h>

/*
 * block/section address mask and size definitions.
 */

/* PAGE_SHIFT determines the page size */
#undef  PAGE_SIZE
#define PAGE_SHIFT		12
#define PAGE_SIZE		(1 << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE - 1))

/***************************************************************/

/*
 * Memory types
 */
#define MT_DEVICE_NGNRNE	0
#define MT_DEVICE_NGNRE		1
#define MT_DEVICE_GRE		2
#define MT_NORMAL_NC		3
#define MT_NORMAL		4

#define MEMORY_ATTRIBUTES	((0x00 << (MT_DEVICE_NGNRNE * 8)) |	\
				(0x04 << (MT_DEVICE_NGNRE * 8))   |	\
				(0x0c << (MT_DEVICE_GRE * 8))     |	\
				(0x44 << (MT_NORMAL_NC * 8))      |	\
				(UL(0xff) << (MT_NORMAL * 8)))

/*
 * Hardware page table definitions.
 *
 */

#define PTE_TYPE_MASK		(3 << 0)
#define PTE_TYPE_FAULT		(0 << 0)
#define PTE_TYPE_TABLE		(3 << 0)
#define PTE_TYPE_PAGE		(3 << 0)
#define PTE_TYPE_BLOCK		(1 << 0)
#define PTE_TYPE_VALID		(1 << 0)

#define PTE_TABLE_PXN		(1UL << 59)
#define PTE_TABLE_XN		(1UL << 60)
#define PTE_TABLE_AP		(1UL << 61)
#define PTE_TABLE_NS		(1UL << 63)

/*
 * Block
 */
#define PTE_BLOCK_MEMTYPE(x)	((x) << 2)
#define PTE_BLOCK_NS            (1 << 5)
#define PTE_BLOCK_NON_SHARE	(0 << 8)
#define PTE_BLOCK_OUTER_SHARE	(2 << 8)
#define PTE_BLOCK_INNER_SHARE	(3 << 8)
#define PTE_BLOCK_AF		(1 << 10)
#define PTE_BLOCK_NG		(1 << 11)
#define PTE_BLOCK_PXN		(UL(1) << 53)
#define PTE_BLOCK_UXN		(UL(1) << 54)

/*
 * AttrIndx[2:0]
 */
#define PMD_ATTRINDX(t)		((t) << 2)
#define PMD_ATTRINDX_MASK	(7 << 2)
#define PMD_ATTRMASK		(PTE_BLOCK_PXN		| \
				 PTE_BLOCK_UXN		| \
				 PMD_ATTRINDX_MASK	| \
				 PTE_TYPE_VALID)

/*
 * TCR flags.
 */
#define TCR_T0SZ(x)		((64 - (x)) << 0)
#define TCR_IRGN_NC		(0 << 8)
#define TCR_IRGN_WBWA		(1 << 8)
#define TCR_IRGN_WT		(2 << 8)
#define TCR_IRGN_WBNWA		(3 << 8)
#define TCR_IRGN_MASK		(3 << 8)
#define TCR_ORGN_NC		(0 << 10)
#define TCR_ORGN_WBWA		(1 << 10)
#define TCR_ORGN_WT		(2 << 10)
#define TCR_ORGN_WBNWA		(3 << 10)
#define TCR_ORGN_MASK		(3 << 10)
#define TCR_SHARED_NON		(0 << 12)
#define TCR_SHARED_OUTER	(2 << 12)
#define TCR_SHARED_INNER	(3 << 12)
#define TCR_TG0_4K		(0 << 14)
#define TCR_TG0_64K		(1 << 14)
#define TCR_TG0_16K		(2 << 14)
#define TCR_EPD1_DISABLE	(1 << 23)

#define TCR_EL1_RSVD		(1 << 31)
#define TCR_EL2_RSVD		(1 << 31 | 1 << 23)
#define TCR_EL3_RSVD		(1 << 31 | 1 << 23)

#ifndef __ASSEMBLY__
static inline void set_ttbr_tcr_mair(int el, u64 table, u64 tcr, u64 attr)
{
	asm volatile("dsb sy");
	if (el == 1) {
		asm volatile("msr ttbr0_el1, %0" : : "r" (table) : "memory");
		asm volatile("msr tcr_el1, %0" : : "r" (tcr) : "memory");
		asm volatile("msr mair_el1, %0" : : "r" (attr) : "memory");
	} else if (el == 2) {
		asm volatile("msr ttbr0_el2, %0" : : "r" (table) : "memory");
		asm volatile("msr tcr_el2, %0" : : "r" (tcr) : "memory");
		asm volatile("msr mair_el2, %0" : : "r" (attr) : "memory");
	} else if (el == 3) {
		asm volatile("msr ttbr0_el3, %0" : : "r" (table) : "memory");
		asm volatile("msr tcr_el3, %0" : : "r" (tcr) : "memory");
		asm volatile("msr mair_el3, %0" : : "r" (attr) : "memory");
	} else {
		hang();
	}
	asm volatile("isb");
}

struct mm_region {
	u64 virt;
	u64 phys;
	u64 size;
	u64 attrs;
};

extern struct mm_region *mem_map;
void setup_pgtables(void);
u64 get_tcr(int el, u64 *pips, u64 *pva_bits);
#endif

#endif /* _ASM_ARMV8_MMU_H_ */
