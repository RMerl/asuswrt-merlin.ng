/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef __ASM_MACH_TLB_H
#define __ASM_MACH_TLB_H

#include <asm/mipsregs.h>
#include <linux/bitops.h>
#include <mach/common.h>
#include <linux/sizes.h>

#define TLB_HI_MASK      0xffffe000
#define TLB_LO_MASK      0x3fffffff	/* Masks off Fill bits */
#define TLB_LO_SHIFT     6	/* PFN Start bit */

#define PAGEMASK_SHIFT   13

#define MMU_PAGE_CACHED   (3 << 3)	/* C(5:3) Cache Coherency Attributes */
#define MMU_PAGE_UNCACHED (2 << 3)	/* C(5:3) Cache Coherency Attributes */
#define MMU_PAGE_DIRTY    BIT(2)	/* = Writeable */
#define MMU_PAGE_VALID    BIT(1)
#define MMU_PAGE_GLOBAL   BIT(0)
#define MMU_REGIO_RO_C    (MMU_PAGE_CACHED | MMU_PAGE_VALID | MMU_PAGE_GLOBAL)
#define MMU_REGIO_RO      (MMU_PAGE_UNCACHED | MMU_PAGE_VALID | MMU_PAGE_GLOBAL)
#define MMU_REGIO_RW      (MMU_PAGE_DIRTY | MMU_REGIO_RO)
#define MMU_REGIO_INVAL   (MMU_PAGE_GLOBAL)

#define TLB_COUNT_MASK	  GENMASK(5, 0)
#define TLB_COUNT_OFF	  25

static inline u32 get_tlb_count(void)
{
	register u32 config1;

	config1 = read_c0_config1();
	config1 >>= TLB_COUNT_OFF;
	config1 &= TLB_COUNT_MASK;

	return 1 + config1;
}

static inline void create_tlb(int index, u32 offset, u32 size, u32 tlb_attrib1,
			      u32 tlb_attrib2)
{
	register u32 tlb_mask, tlb_lo0, tlb_lo1;

	tlb_mask = ((size >> 12) - 1) << PAGEMASK_SHIFT;
	tlb_lo0 = tlb_attrib1 | (offset >> TLB_LO_SHIFT);
	tlb_lo1 = tlb_attrib2 | ((offset + size) >> TLB_LO_SHIFT);

	write_one_tlb(index, tlb_mask, offset & TLB_HI_MASK,
		      tlb_lo0 & TLB_LO_MASK, tlb_lo1 & TLB_LO_MASK);
}
#endif				/* __ASM_MACH_TLB_H */
