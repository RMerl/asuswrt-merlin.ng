/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2022 Broadcom Ltd.
 */
#include <common.h>
#include "mmu_map_v7.h"
#ifdef CONFIG_SPL_XIP_SUPPORT
#include <boot_flash.h>
#endif

static struct mm_region broadcom_bcm96764_mem_map[] = {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* 
	 *DDR memory. Static mapping for lower 2GB. Upper 2GB is mapped
	  at run time. Use Device Attr to avoid CPU speculative fetch.
	*/
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 2UL * SZ_1G,
		.attrs = SECTION_ATTR_DEVICE,
	},
	/* LMEM for bootrom runtime  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR,
		.phys = CONFIG_SYS_INIT_RAM_ADDR,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_CACHED_MEM,
	},

#ifdef CONFIG_SPL_XIP_SUPPORT
	/* NOR FLASH XIP Window */
	{
		.virt = NOR_XIP_BASE_ADDR,
		.phys = NOR_XIP_BASE_ADDR,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},
#endif
#else
	/*
	 * uboot ddr entries for cached memory will be set in ram_bank_mmu_setup
	 * based on actual size
	 */

	{
		/* PMC */
		.virt = 0xffa00000UL,
		.phys = 0xffa00000UL,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},
#endif
	/* MEMC control registers */
	{
		.virt = 0x80040000UL,
		.phys = 0x80040000UL,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},

	{
		/* SoC peripheral */
		.virt = 0xff800000UL,
		.phys = 0xff800000UL,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},
		/* UBUS */
	{
		.virt = 0x83000000,
		.phys = 0x83000000,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},	
		/* UBUS CCB */
	{
		.virt = 0x81200000,
		.phys = 0x81200000,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},
		/* GIC, BIUCFG */
	{
		.virt = 0x81000000UL,
		.phys = 0x81000000UL,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},

	/* SNP, SWITCH SYSPORT */
	{
		.virt = 0x80200000UL,
		.phys = 0x80200000UL,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE
	},

	{
		/* BLUT */
		.virt = 0xffe00000,
		.phys = 0xffe00000,
		.size = SZ_2M,
		.attrs = SECTION_ATTR_DEVICE,
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm96764_mem_map;
