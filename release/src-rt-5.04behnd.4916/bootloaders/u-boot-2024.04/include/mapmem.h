/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 National Instruments
 *
 * (C) Copyright 2015
 * Joe Hershberger <joe.hershberger@ni.com>
 */

#ifndef __MAPMEM_H
#define __MAPMEM_H

/* Define a null map_sysmem() if the architecture doesn't use it */
# ifdef CONFIG_ARCH_MAP_SYSMEM
#include <asm/io.h>
# else
#include <linux/types.h>

static inline void *map_sysmem(phys_addr_t paddr, unsigned long len)
{
	return (void *)(uintptr_t)paddr;
}

static inline void unmap_sysmem(const void *vaddr)
{
}

static inline phys_addr_t map_to_sysmem(const void *ptr)
{
	return (phys_addr_t)(uintptr_t)ptr;
}

/**
 * nomap_sysmem() - pass through an address unchanged
 *
 * This is used to indicate an address which should NOT be mapped, e.g. in
 * SMBIOS tables. Using this function instead of a case shows that the sandbox
 * conversion has been done
 */
static inline void *nomap_sysmem(phys_addr_t paddr, unsigned long len)
{
	return (void *)(uintptr_t)paddr;
}

static inline phys_addr_t nomap_to_sysmem(const void *ptr)
{
	return (phys_addr_t)(uintptr_t)ptr;
}

# endif

#endif /* __MAPMEM_H */
