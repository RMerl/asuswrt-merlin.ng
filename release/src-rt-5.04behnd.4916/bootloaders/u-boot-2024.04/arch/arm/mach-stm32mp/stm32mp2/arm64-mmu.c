// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <mach/stm32.h>

#define MP2_MEM_MAP_MAX 10

#if (CONFIG_TEXT_BASE < STM32_DDR_BASE) || \
	(CONFIG_TEXT_BASE > (STM32_DDR_BASE + STM32_DDR_SIZE))
#error "invalid CONFIG_TEXT_BASE value"
#endif

struct mm_region stm32mp2_mem_map[MP2_MEM_MAP_MAX] = {
	{
		/* PCIe */
		.virt = 0x10000000UL,
		.phys = 0x10000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* LPSRAMs, VDERAM, RETRAM, SRAMs, SYSRAM: alias1 */
		.virt = 0x20000000UL,
		.phys = 0x20000000UL,
		.size = 0x00200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Peripherals: alias1 */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* OSPI and FMC: memory-map area */
		.virt = 0x60000000UL,
		.phys = 0x60000000UL,
		.size = 0x20000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/*
		 * DDR = STM32_DDR_BASE / STM32_DDR_SIZE
		 * the beginning of DDR (before CONFIG_TEXT_BASE) is not
		 * mapped, protected by RIF and reserved for other firmware
		 * (OP-TEE / TF-M / Cube M33)
		 */
		.virt = CONFIG_TEXT_BASE,
		.phys = CONFIG_TEXT_BASE,
		.size = STM32_DDR_SIZE - (CONFIG_TEXT_BASE - STM32_DDR_BASE),
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = stm32mp2_mem_map;
