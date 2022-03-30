// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <asm/armv8/mmu.h>

/* Armada 7k/8k */
#define MVEBU_RFU_BASE			(MVEBU_REGISTER(0x6f0000))
#define RFU_GLOBAL_SW_RST		(MVEBU_RFU_BASE + 0x84)
#define RFU_SW_RESET_OFFSET		0

#define SAR0_REG			(MVEBU_REGISTER(0x2400200))
#define BOOT_MODE_MASK			0x3f
#define BOOT_MODE_OFFSET		4

/*
 * The following table includes all memory regions for Armada 7k and
 * 8k SoCs. The Armada 7k is missing the CP110 slave regions here. Lets
 * define these regions at the beginning of the struct so that they
 * can be easier removed later dynamically if an Armada 7k device is detected.
 * For a detailed memory map, please see doc/mvebu/armada-8k-memory.txt
 */
#define ARMADA_7K8K_COMMON_REGIONS_START	2
static struct mm_region mvebu_mem_map[] = {
	/* Armada 80x0 memory regions include the CP1 (slave) units */
	{
		/* SRAM, MMIO regions - CP110 slave region */
		.phys = 0xf4000000UL,
		.virt = 0xf4000000UL,
		.size = 0x02000000UL,	/* 32MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* PCI CP1 regions */
		.phys = 0xfa000000UL,
		.virt = 0xfa000000UL,
		.size = 0x04000000UL,	/* 64MiB CP110 slave PCI space */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	/* Armada 80x0 and 70x0 common memory regions start here */
	{
		/* RAM */
		.phys = 0x0UL,
		.virt = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		/* SRAM, MMIO regions - AP806 region */
		.phys = 0xf0000000UL,
		.virt = 0xf0000000UL,
		.size = 0x01000000UL,	/* 16MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* SRAM, MMIO regions - CP110 master region */
		.phys = 0xf2000000UL,
		.virt = 0xf2000000UL,
		.size = 0x02000000UL,	/* 32MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		/* PCI CP0 regions */
		.phys = 0xf6000000UL,
		.virt = 0xf6000000UL,
		.size = 0x04000000UL,	/* 64MiB CP110 master PCI space */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = mvebu_mem_map;

void enable_caches(void)
{
	/*
	 * Armada 7k is not equipped with the CP110 slave CP. In case this
	 * code runs on an Armada 7k device, lets remove the CP110 slave
	 * entries from the memory mapping by moving the start to the
	 * common regions.
	 */
	if (of_machine_is_compatible("marvell,armada7040"))
		mem_map = &mvebu_mem_map[ARMADA_7K8K_COMMON_REGIONS_START];

	icache_enable();
	dcache_enable();
}

void reset_cpu(ulong ignored)
{
	u32 reg;

	reg = readl(RFU_GLOBAL_SW_RST);
	reg &= ~(1 << RFU_SW_RESET_OFFSET);
	writel(reg, RFU_GLOBAL_SW_RST);
}

/*
 * TODO - implement this functionality using platform
 *        clock driver once it gets available
 * Return NAND clock in Hz
 */
u32 mvebu_get_nand_clock(void)
{
	unsigned long NAND_FLASH_CLK_CTRL = 0xF2440700UL;
	unsigned long NF_CLOCK_SEL_MASK = 0x1;
	u32 reg;

	reg = readl(NAND_FLASH_CLK_CTRL);
	if (reg & NF_CLOCK_SEL_MASK)
		return 400 * 1000000;
	else
		return 250 * 1000000;
}

int mmc_get_env_dev(void)
{
	u32 reg;
	unsigned int boot_mode;

	reg = readl(SAR0_REG);
	boot_mode = (reg >> BOOT_MODE_OFFSET) & BOOT_MODE_MASK;

	switch (boot_mode) {
	case 0x28:
	case 0x2a:
		return 0;
	case 0x29:
	case 0x2b:
		return 1;
	}

	return CONFIG_SYS_MMC_ENV_DEV;
}
