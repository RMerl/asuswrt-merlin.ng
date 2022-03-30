/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#ifndef MMU_MAP_V7_H
#define MMU_MAP_V7_H


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
#define SECTION_SET(__PA__,__ATTR__) ((__PA__&0xfff00000)|(__ATTR__&0xfffff))

struct mm_region {
	phys_addr_t phys;
#ifdef CONFIG_ARMV7_LPAE
	uint64_t virt;
	uint64_t attrs;
#else
	uint32_t virt;
	uint32_t attrs;
#endif
	uint32_t size;
};

extern struct mm_region *mem_map;

#endif
