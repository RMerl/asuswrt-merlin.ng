/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#include <common.h>
#include "mmu_map_v7.h"

static struct mm_region broadcom_bcm963138_mem_map[] = {
#if defined(CONFIG_SPL_BUILD)
#if !defined(CONFIG_TPL_BUILD)
	/* SPL table */

	/* DDR memory. Enable the maximum 1GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
	{
		.virt = 0x00000000,
		.phys = 0x00000000,
		.size = SZ_1G,
		.attrs = SECTION_ATTR_DEVICE, 
	},
	/* LMEM for SPL runtime  */
	{
		.virt = 0x80700000,
		.phys = 0x80700000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_CACHED_MEM,
	},
#else
	/* TPL table */
	/* 
	 * uboot ddr entries for cached memory will be set in ram_bank_mmu_setup 
	 * based on actual size 
         */
	/* LMEM for SPL runtime  */
	{
		.virt = 0x80700000,
		.phys = 0x80700000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_CACHED_MEM,
	},
#endif
#else
	/* u-boot table */
	/* 
	 * uboot ddr entries for cached memory will be set in ram_bank_mmu_setup 
	 * based on actual size 
         */
	{
		/* APM */
		.virt = 0x80100000,
		.phys = 0x80100000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* RDP */
		.virt = 0x80200000,
		.phys = 0x80200000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
#endif
	{
		/* Register space (covers MEMC, USB, etc) */
		.virt = 0x80000000,
		.phys = 0x80000000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* PMC */
		.virt = 0x80400000,
		.phys = 0x80400000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* SoC peripheral */
		.virt = 0xfff00000,
		.phys = 0xfff00000,
                .size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	}, 
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm963138_mem_map;
