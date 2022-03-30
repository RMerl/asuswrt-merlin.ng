// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Common functions for OMAP4/5 based boards
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *	Aneesh V	<aneesh@ti.com>
 *	Steve Sakoman	<steve@sakoman.com>
 */

#include <common.h>
#include <asm/cache.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Without LPAE short descriptors are used
 * Set C - Cache Bit3
 * Set B - Buffer Bit2
 * The last 2 bits set to 0b10
 * Do Not set XN bit4
 * So value is 0xe
 *
 * With LPAE cache configuration happens via MAIR0 register
 * AttrIndx value is 0x3 for picking byte3 for MAIR0 which has 0xFF.
 * 0xFF maps to Cache writeback with Read and Write Allocate set
 * The bits[1:0] should have the value 0b01 for the first level
 * descriptor.
 * So the value is 0xd
 */

#ifdef CONFIG_ARMV7_LPAE
#define ARMV7_DCACHE_POLICY	DCACHE_WRITEALLOC
#else
#define ARMV7_DCACHE_POLICY	DCACHE_WRITEBACK & ~TTB_SECT_XN_MASK
#endif

#define ARMV7_DOMAIN_CLIENT	1
#define ARMV7_DOMAIN_MASK	(0x3 << 0)

void enable_caches(void)
{

	/* Enable I cache if not enabled */
	if (!icache_status())
		icache_enable();

	dcache_enable();
}

void dram_bank_mmu_setup(int bank)
{
	bd_t *bd = gd->bd;
	int	i;

	u32 start = bd->bi_dram[bank].start >> MMU_SECTION_SHIFT;
	u32 size = bd->bi_dram[bank].size >> MMU_SECTION_SHIFT;
	u32 end = start + size;

	debug("%s: bank: %d\n", __func__, bank);
	for (i = start; i < end; i++)
		set_section_dcache(i, ARMV7_DCACHE_POLICY);
}

void arm_init_domains(void)
{
	u32 reg;

	reg = get_dacr();
	/*
	* Set DOMAIN to client access so that all permissions
	* set in pagetables are validated by the mmu.
	*/
	reg &= ~ARMV7_DOMAIN_MASK;
	reg |= ARMV7_DOMAIN_CLIENT;
	set_dacr(reg);
}
