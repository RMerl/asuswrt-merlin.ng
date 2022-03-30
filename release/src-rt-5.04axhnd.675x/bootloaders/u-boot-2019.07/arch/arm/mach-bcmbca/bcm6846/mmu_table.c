/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#include <common.h>
#include "mmu_map_v7.h"

static struct mm_region broadcom_bcm96846_mem_map[] = {
#if defined(CONFIG_SPL_BUILD) 
#if !defined(CONFIG_TPL_BUILD)
	/* SPL table */
	/* DDR memory. Enable the maximum 2GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
	{
		.virt = 0x00000000,
		.phys = 0x00000000,
		.size = SZ_1G,
		.attrs = SECTION_ATTR_DEVICE, 
	},
	/* PSRAM for SPL runtime  */

	{
		.virt = 0x82600000,
		.phys = 0x82600000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},

    /* page table area */
    {
        .virt = 0x7ff00000,
        .phys = 0x7ff00000,
        .size = SZ_1M,
        .attrs = SECTION_ATTR_DEVICE,
    },
	/* STD Mem  */
	{
		.virt = CONFIG_SYS_INIT_STD_32K_ADDR,
		.phys = CONFIG_SYS_INIT_STD_32K_ADDR,
		.size = SZ_32K,
		.attrs = SECTION_ATTR_DEVICE,
	},
#else
	/* TPL table */
	/* 
	 * uboot ddr entries for cached memory will be set in ram_bank_mmu_setup 
	 * based on actual size 
         */
	/* STD Mem  */
	{
		.virt = CONFIG_SYS_INIT_STD_32K_ADDR,
		.phys = CONFIG_SYS_INIT_STD_32K_ADDR,
		.size = SZ_32K,
		.attrs = SECTION_ATTR_DEVICE,
	},
    {
        /* UBUS */
        .virt = 0x83000000,
        .phys = 0x83000000,
        .size = SZ_1M,
        .attrs = SECTION_ATTR_DEVICE,
    },

#endif
#else
	/* u-boot table */
	/* 
	 * uboot ddr entries for cached memory will be set in ram_bank_mmu_setup 
	 * based on actual size 
         */
	{
		/* BIU & CCI400 */
		.virt = 0x81000000,
		.phys = 0x81000000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
		/* USB */
	{
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
	    /* UBUS */
	    .virt = 0x83000000,
	    .phys = 0x83000000,
	    .size = SZ_1M,
	    .attrs = SECTION_ATTR_DEVICE,
	},
#endif
	{
		/* PMC */
		.virt = 0xffb00000,
		.phys = 0xffb00000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* MEMC PHY register */
		.virt = 0x80100000,
		.phys = 0x80100000,
		.size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* SoC peripheral */
		.virt = 0xff800000,
		.phys = 0xff800000,
                .size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* BLUT */
		.virt = 0xfff00000,
		.phys = 0xfff00000,
                .size = SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	}, 
		/* XRDP */
	{
		.virt = 0x82000000UL,
		.phys = 0x82000000UL,
		.size = 16 * SZ_1M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm96846_mem_map;
