/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */
#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region broadcom_bcm94908_mem_map[] = {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* DDR memory. Enable the maximum 4GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 2UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* MEMC and DDRY PHY control registers */
	{
		.virt = 0x80018000UL,
		.phys = 0x80018000UL,
		.size = 0x40000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* RDP MEM for bootrom runtime  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_128K_OFFSET,
		.phys = CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_128K_OFFSET,
		.size = SZ_128K,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	/* RDP MEM 32Kfor bootrom runtime  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR_VIRT,
		.phys = CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_32K_OFFSET,
		.size = SZ_32K,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	/* RDP MEM 48K part 0 bootrom runtime  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR_VIRT + SZ_32K,
		.phys = CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_48K_0_OFFSET,
		.size = SZ_32K + SZ_16K,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	/* RDP MEM 48K part 1 bootrom runtime  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR_VIRT + SZ_32K + SZ_32K + SZ_16K,
		.phys = CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_48K_1_OFFSET,
		.size = SZ_32K + SZ_16K,
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
		/* PMC */
	{
		.virt = 0x80200000,
		.phys = 0x80200000,
		.size = 0x81000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
		/* USB */
	{
		.virt = 0x8000C000UL,
		.phys = 0x8000C000UL,
		.size = 0x3000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

		/* BIUCFG */
	{
		.virt = 0x81062000UL,
		.phys = 0x81062000UL,
		.size = 0x1000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

		/* BOOTLUT */
	{
		.virt = 0xffff0000UL,
		.phys = 0xffff0000UL,
		.size = 0x1000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

		/* UNIMAC */
	{
		.virt = 0x80002000UL,
		.phys = 0x80002000UL,
		.size = 0x1000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

		/* SF2 */
	{
		.virt = 0x80080000UL,
		.phys = 0x80080000UL,
		.size = 0x42000,
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
	{
		/* SoC peripheral */
		.virt = 0xff800000UL,
		.phys = 0xff800000UL,
		.size = 0x100000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, 
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm94908_mem_map;
