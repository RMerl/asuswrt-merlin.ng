/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef __ASM_MACH_MSCC_IOREMAP_H
#define __ASM_MACH_MSCC_IOREMAP_H

#include <linux/types.h>
#include <mach/common.h>

/*
 * Allow physical addresses to be fixed up to help peripherals located
 * outside the low 32-bit range -- generic pass-through version.
 */
static inline phys_addr_t fixup_bigphys_addr(phys_addr_t phys_addr,
					     phys_addr_t size)
{
	return phys_addr;
}

static inline int is_vcoreiii_internal_registers(phys_addr_t offset)
{
	if ((offset >= MSCC_IO_ORIGIN1_OFFSET &&
	     offset < (MSCC_IO_ORIGIN1_OFFSET + MSCC_IO_ORIGIN1_SIZE)) ||
	    (offset >= MSCC_IO_ORIGIN2_OFFSET &&
	     offset < (MSCC_IO_ORIGIN2_OFFSET + MSCC_IO_ORIGIN2_SIZE)))
		return 1;

	return 0;
}

static inline void __iomem *plat_ioremap(phys_addr_t offset, unsigned long size,
					 unsigned long flags)
{
	if (is_vcoreiii_internal_registers(offset))
		return (void __iomem *)offset;

	return NULL;
}

static inline int plat_iounmap(const volatile void __iomem *addr)
{
	return is_vcoreiii_internal_registers((unsigned long)addr);
}

#define _page_cachable_default	_CACHE_CACHABLE_NONCOHERENT

#endif				/* __ASM_MACH_MSCC_IOREMAP_H */
