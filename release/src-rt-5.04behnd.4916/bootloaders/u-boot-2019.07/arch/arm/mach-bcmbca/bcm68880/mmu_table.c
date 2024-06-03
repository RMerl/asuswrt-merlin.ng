/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2021 Broadcom Ltd.
 */
#include <common.h>
#include <asm/armv8/mmu.h>

static struct mm_region broadcom_bcm968880_mem_map[] = {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* DDR memory. Enable the maximum 4GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 1UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
					PTE_BLOCK_INNER_SHARE
	},
	/* PSRAM  */
	{
		.virt = CONFIG_SYS_INIT_RAM_ADDR,
		.phys = CONFIG_SYS_INIT_RAM_ADDR,
		.size = CONFIG_SYS_INIT_RAM_SIZE,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
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

		/* XRDP */
	{
		.virt = 0x82000000UL,
		.phys = 0x82000000UL,
		.size = 16 * SZ_1M,
                .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
                 PTE_BLOCK_NON_SHARE |
                 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

		/* PMC */
	{
		.virt = 0xffb00000UL,
		.phys = 0xffb00000UL,
		.size = 0x6000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
		/* PROC_MON */
	{
		.virt = 0xffb20000UL,
		.phys = 0xffb20000UL,
		.size = 0x1000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
		/* DTCM */
	{
		.virt = 0xffb80000UL,
		.phys = 0xffb80000UL,
		.size = 0x1000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
		/* ITCM */
	{
		.virt = 0xffbc0000UL,
		.phys = 0xffbc0000UL,
		.size = 0x4000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

    {
        .virt = 0x81001000UL,
        .phys = 0x81001000UL,
        .size = 0x7000,
        .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
             PTE_BLOCK_NON_SHARE |
             PTE_BLOCK_PXN | PTE_BLOCK_UXN
    },
		/* CCI500 */
	{
		.virt = 0x81100000,
		.phys = 0x81100000,
		.size = 0xa0000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* CCB */
	{
		.virt = 0x81200000,
		.phys = 0x81200000,
		.size = 0x6000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
		 PTE_BLOCK_NON_SHARE |
		 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
        /* UBUS */
    {
        .virt = 0x83000000UL,
        .phys = 0x83000000UL,
        .size = 0xd0000,
        .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
             PTE_BLOCK_NON_SHARE |
             PTE_BLOCK_PXN | PTE_BLOCK_UXN
    },
        /* ETH */
    {
        .virt = 0x83400000UL,
        .phys = 0x83400000UL,
        .size = 0x400000,
        .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
             PTE_BLOCK_NON_SHARE |
             PTE_BLOCK_PXN | PTE_BLOCK_UXN
    },
        /* PG_BUS */
    {
        .virt = 0x84000000UL,
        .phys = 0x84000000UL,
        .size = 0x800000,
        .attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
             PTE_BLOCK_NON_SHARE |
             PTE_BLOCK_PXN | PTE_BLOCK_UXN
    },

#endif
        /* BIUCFG */
    {
        .virt = 0x81060000UL,
        .phys = 0x81060000UL,
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
		/* SoC peripheral */
		.virt = 0xff800000UL,
		.phys = 0xff800000UL,
		.size = SZ_1M,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, 
    {
		/* SoC RG peripheral */
		.virt = 0x84000000UL,
		.phys = 0x84000000UL,
		.size = SZ_8M,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = broadcom_bcm968880_mem_map;
