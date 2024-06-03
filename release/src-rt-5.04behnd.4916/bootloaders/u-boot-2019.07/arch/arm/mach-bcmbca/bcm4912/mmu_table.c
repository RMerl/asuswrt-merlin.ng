/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#include <common.h>
#include <asm/armv8/mmu.h>
#ifdef CONFIG_SPL_XIP_SUPPORT
#include <boot_flash.h>
#endif

static struct mm_region broadcom_bcm94912_mem_map[] = {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* DDR memory. Enable the maximum 8GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 2UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x100000000UL,
		.phys = 0x100000000UL,
		.size = 6UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

	/* LMEM for bootrom runtime  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR,
		.phys = CONFIG_SYS_INIT_RAM_ADDR,
		.size = CONFIG_SYS_INIT_RAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	/* STD Mem  */
	{
		.virt = CONFIG_SYS_INIT_STD_32K_ADDR,
		.phys = CONFIG_SYS_INIT_STD_32K_ADDR,
		.size = SZ_32K,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

#ifdef CONFIG_SPL_XIP_SUPPORT
	/* NOR FLASH XIP Window */
	{
		.virt = NOR_XIP_BASE_ADDR,
		.phys = NOR_XIP_BASE_ADDR,
		.size = 0x100000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
#else
	/* DDR entries for cached memory, total size is a placehold
	   and will be filled in at run time. MUST be first entry */
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 1UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
		/* GIC */
	{
		.virt = 0x81001000UL,
		.phys = 0x81001000UL,
		.size = 0x7000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},


	{
		/* XRDP PSRAM */
		.virt = 0x82000000UL,
		.phys = 0x82000000UL,
		.size = 0x30000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},

	{
		/* XRDP Rest of block */
		.virt = 0x82700000UL,
		.phys = 0x82700000UL,
		.size = 0x600000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

	{
		/* ETH_PHY_TOP */
		.virt = 0x83400000UL,
		.phys = 0x83400000UL,
		.size = 0x400000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

	{
		/* PMC */
		.virt = 0xffa00000UL,
		.phys = 0xffa00000UL,
		.size = 0x200000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* STD Mem  */
	{
		.virt = CONFIG_SYS_INIT_STD_32K_ADDR,
		.phys = CONFIG_SYS_INIT_STD_32K_ADDR,
		.size = SZ_32K,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	/* MEMC control registers */
	{
		.virt = 0x80040000UL,
		.phys = 0x80040000UL,
		.size = 0x2000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

	/* DDR PHY control registers */
	{
		.virt = 0x80060000UL,
		.phys = 0x80060000UL,
		.size = 0x6000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* DPFE control registers */
	{
		.virt = 0x80070000UL,
		.phys = 0x80070000UL,
		.size = 0xc000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

	{
		/* SoC peripheral */
		.virt = 0xff800000UL,
		.phys = 0xff800000UL,
                .size = 0x100000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
		/* CCI-500 */
	{
		.virt = 0x81100000,
		.phys = 0x81100000,
		.size = 0x91000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},	
		/* BIUCFG */
	{
		.virt = 0x81060000,
		.phys = 0x81060000,
		.size = 0x3000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

		/* UBUS4 Coherency Port */
	{
		.virt = 0x810A0000,
		.phys = 0x810A0000,
		.size = 0x1000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm94912_mem_map;
