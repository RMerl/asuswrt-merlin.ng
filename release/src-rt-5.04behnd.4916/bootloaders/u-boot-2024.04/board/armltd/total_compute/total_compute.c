// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Arm Limited
 * Usama Arif <usama.arif@arm.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/platform_data/serial_pl01x.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>

static const struct pl01x_serial_plat serial_plat = {
	.base = UART0_BASE,
	.type = TYPE_PL011,
	.clock = CFG_PL011_CLOCK,
};

U_BOOT_DRVINFO(total_compute_serials) = {
	.name = "serial_pl01x",
	.plat = &serial_plat,
};

static struct mm_region total_compute_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		.virt = 0x80000000UL,
		.phys = 0x80000000UL,
		.size = 0xff80000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = total_compute_mem_map;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return 0;
}

/* Nothing to be done here as handled by PSCI interface */
void reset_cpu(void)
{
}
