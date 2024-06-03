/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2022 Broadcom Ltd.
 */
#include <common.h>
#include "mmu_map_v7.h"
#ifdef CONFIG_SPL_XIP_SUPPORT
#include <boot_flash.h>
#endif

static struct mm_region broadcom_bcm96766_mem_map[] = {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* DDR memory. Enable the maximum 8GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
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
		.size = CONFIG_SYS_INIT_RAM_SIZE,
		.attrs = SECTION_ATTR_CACHED_MEM, 
	},
	/* STD Mem  */
	{
		.virt = CONFIG_SYS_INIT_STD_32K_ADDR,
		.phys = CONFIG_SYS_INIT_STD_32K_ADDR,
		.size = SZ_32K,
		.attrs = SECTION_ATTR_CACHED_MEM,
	},

#ifdef CONFIG_SPL_XIP_SUPPORT
	/* NOR FLASH XIP Window */
	{
		.virt = NOR_XIP_BASE_ADDR,
		.phys = NOR_XIP_BASE_ADDR,
		.size = 0x100000,
		.attrs = SECTION_ATTR_DEVICE,
	},
#endif
#else
		/* GIC */
	{
		.virt = 0x81001000UL,
		.phys = 0x81001000UL,
		.size = 0x7000,
		.attrs = SECTION_ATTR_DEVICE,
	},

	{
		/* PMC */
		.virt = 0xffa00000UL,
		.phys = 0xffa00000UL,
		.size = 0x200000,
		.attrs = SECTION_ATTR_DEVICE,
	},
	/* STD Mem  */
	{
		.virt = CONFIG_SYS_INIT_STD_32K_ADDR,
		.phys = CONFIG_SYS_INIT_STD_32K_ADDR,
		.size = SZ_32K,
		.attrs = SECTION_ATTR_DEVICE,
	},
#endif
	/* MEMC control registers */
	{
		.virt = 0x80040000UL,
		.phys = 0x80040000UL,
		.size = 0x4000,
		.attrs = SECTION_ATTR_DEVICE,
	},

	/* DDR PHY registers */
	{
		.virt = 0x80060000UL,
		.phys = 0x80060000UL,
		.size = 0x20000,
		.attrs = SECTION_ATTR_DEVICE,
	},
	
	{
		/* SoC peripheral */
		.virt = 0xff800000UL,
		.phys = 0xff800000UL,
		.size = 0x100000,
		.attrs = SECTION_ATTR_DEVICE,
	},
		/* UBUS CCB */
	{
		.virt = 0x81200000,
		.phys = 0x81200000,
		.size = 0x6000,
		.attrs = SECTION_ATTR_DEVICE,
	},
		/* BIUCFG */
	{
		.virt = 0x81060000,
		.phys = 0x81060000,
		.size = 0x3000,
		.attrs = SECTION_ATTR_DEVICE,
	},
	/* CNP */
	{
		.virt = 0x80200000UL,
		.phys = 0x80200000UL,
		.size = 0x300000,
		.attrs = SECTION_ATTR_DEVICE
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm96766_mem_map;
