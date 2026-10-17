// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 SberDevices, Inc.
 */

#include <common.h>
#include <asm/arch/a1.h>
#include <asm/arch/boot.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <linux/sizes.h>

phys_size_t get_effective_memsize(void)
{
	return ((readl(A1_SYSCTRL_SEC_STATUS_REG4) & A1_SYSCTRL_MEM_SIZE_MASK)
		>> A1_SYSCTRL_MEM_SIZE_SHIFT) * SZ_1M;
}

void meson_init_reserved_memory(__maybe_unused void *fdt)
{
}

int meson_get_boot_device(void)
{
	return -ENOSYS;
}

static struct mm_region a1_mem_map[] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0x7FE00000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/*
		 * This mem region contains in/out shared memory with bl31,
		 * hence it's marked as NORMAL memory type
		 */
		.virt = 0xFFE00000UL,
		.phys = 0xFFE00000UL,
		.size = 0x00200000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = a1_mem_map;
