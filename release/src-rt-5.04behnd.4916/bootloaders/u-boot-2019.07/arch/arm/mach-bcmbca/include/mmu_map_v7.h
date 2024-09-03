/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef MMU_MAP_V7_H
#define MMU_MAP_V7_H

#include <asm/system.h>

#ifdef CONFIG_ARMV7_LPAE
/* Long-Descriptor Block Descriptor Level 1/2 definition */
#define SECTION_XN          (1ULL << 54)
#define SECTION_PXN         (1ULL << 53)
#define SECTION_CONT_HINT   (1ULL << 52)
#define SECTION_NG          (1 << 11)
#define SECTION_AF          (1 << 10)
#define SECTION_SH_SHIFT    8
#define SECTION_SH_NS       (0x0 << SECTION_SH_SHIFT)
#define SECTION_SH_OUTER    (0x2 << SECTION_SH_SHIFT)
#define SECTION_SH_INNER    (0x3 << SECTION_SH_SHIFT)
#define SECTION_AP_SHIFT    6
#define SECTION_AP_RW_PL1   (0x0 << SECTION_AP_SHIFT)
#define SECTION_AP_RW_ANY   (0x1 << SECTION_AP_SHIFT)
#define SECTION_AP_RO_PL1   (0x2 << SECTION_AP_SHIFT)
#define SECTION_AP_RO_ANY   (0x3 << SECTION_AP_SHIFT)
#define SECTION_NS          (1 << 5)
#define SECTION_MAIR(x)     ((x & 0x7) << 2) /* Index into MAIR */

/*
 * Memory region attributes for LPAE:(defined in arch/arm/include/asm/system.h
 *
 *   n = AttrIndx[2:0]
 *
 *                  n   MAIR
 *   SO memory     000  00000000
 *   WRITETHROUGH  001  10001000
 *   WRITEBACK     010  11001100
 *   WRITEALLOC    011  11111111
 *   NC memory     100  01000100
 *   DEV memory    101  00000100
 *   unused        110  00000000
 *   unused        111  00000000
 */

#define SECTION_ATTR_INVALID       0x0
#define SECTION_ATTR_CACHED_MEM    \
	(TTB_SECT | TTB_SECT_AF | SECTION_MAIR(3))
#define SECTION_ATTR_NONCACHED_MEM \
	(TTB_SECT | TTB_SECT_AF | SECTION_MAIR(4))
#define SECTION_ATTR_DEVICE        \
	(TTB_SECT | TTB_SECT_AF | SECTION_MAIR(5) | SECTION_XN | SECTION_PXN)

struct mm_region {
	phys_addr_t phys;
	uint32_t virt;
	uint64_t attrs;
	uint32_t size;
};
#else
/*  MMU and TT (Translation Tables) definitions

   WBWA == Write-Back, Write-Allocate
   WBNWA == Write-Back, No Write-Allocate
   WTNWA == Write-Through, No Write-Allocate
   NC == Non-Cacheable
   SO == Strongly-Ordered
   SD == Sharable-Device
   NSD == Non-Sharable-Device
*/

#define DESC_DOMAIN(x)          ((x << 5) & 0x000001E0)

// section descriptor definitions
#define SECTION_AP              0xc00
#define SECTION_XN              0x10
#define SECTION_PXN             0x1
#if defined(CONFIG_BCM63138)
/* A9 does not support PXN */
#define SECTION_XN_ALL          (SECTION_XN)
#else
#define SECTION_XN_ALL          (SECTION_XN|SECTION_PXN)
#endif
#define SECTION_SHAREABLE       (1 << 16)
#define SECTION_SUPER_DESC      (1 << 18)
#define SECTION_DESC_NS         (1 << 19)
// TEX[2] = 1
#define SECTION_OUTER_NC_INNER_WBWA         0x00004006
#define SECTION_OUTER_WBNWA_INNER_WBWA      0x00007006
#define SECTION_OUTER_WTNWA_INNER_WBWA      0x00006006
#define SECTION_OUTER_WBWA_INNER_NC         0x00005002
// TEX[2] = 0, OUTER & INNER are same all the time
#define SECTION_OUTER_WBWA_INNER_WBWA       0x0000100E
#define SECTION_OUTER_NSD_INNER_NSD         0x00002002
#define SECTION_OUTER_NC_INNER_NC           0x00001002
#define SECTION_OUTER_WTNWA_INNER_WTNWA     0x0000000A
#define SECTION_OUTER_WBNWA_INNER_WBNWA     0x0000000E
#define SECTION_OUTER_SO_INNER_SO           0x00000002
#define SECTION_OUTER_SD_INNER_SD           0x00000006

// definition for common section attribute
#define SECTION_ATTR_INVALID       0x0
#define SECTION_ATTR_CACHED_MEM    \
	(SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0))
#define SECTION_ATTR_NONCACHED_MEM \
	(SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0))
#define SECTION_ATTR_DEVICE        \
	(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|SECTION_XN_ALL|DESC_DOMAIN(0))
#define SECTION_ATTR_DEVICE_EXEC   \
	(SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0))

struct mm_region {
	phys_addr_t phys;
	uint32_t virt;
	uint32_t attrs;
	uint32_t size;
};

#endif

extern struct mm_region *mem_map;

int map_section(u32 va, phys_addr_t pa, u32 size, u64 attr);
int unmap_section(u32 va, phys_addr_t pa, u32 size);

#endif
