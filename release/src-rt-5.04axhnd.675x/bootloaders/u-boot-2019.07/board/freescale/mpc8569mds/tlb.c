// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/mmu.h>

struct fsl_e_tlb_entry tlb_table[] = {
	/* TLB 0 - for temp stack in cache */
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR, CONFIG_SYS_INIT_RAM_ADDR,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 Initializations */
	/*
	 * TLBe 0:	64M	write-through, guarded
	 * Out of reset this entry is only 4K.
	 * 0xfc000000	32MB	NAND FLASH (CS3)
	 * 0xfe000000	32MB	NOR FLASH (CS0)
	 */
#ifdef CONFIG_NAND_SPL
	SET_TLB_ENTRY(1, CONFIG_SYS_NAND_BASE, CONFIG_SYS_NAND_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_1M, 1),
#else
	SET_TLB_ENTRY(1, CONFIG_SYS_NAND_BASE, CONFIG_SYS_NAND_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_W|MAS2_G,
		      0, 0, BOOKE_PAGESZ_64M, 1),
#endif
	/*
	 * TLBe 1:	256KB	Non-cacheable, guarded
	 * 0xf8000000	32K	BCSR
	 * 0xf8008000	32K	PIB (CS4)
	 * 0xf8010000	32K	PIB (CS5)
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_BCSR_BASE, CONFIG_SYS_BCSR_BASE_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_256K, 1),

	/*
	 * TLBe 2:	256M	Non-cacheable, guarded
	 * 0xa00000000	256M	PCIe MEM (lower half)
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT, CONFIG_SYS_PCIE1_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLBe 3:	256M	Non-cacheable, guarded
	 * 0xb00000000	256M	PCIe MEM (higher half)
	 */
	SET_TLB_ENTRY(1, (CONFIG_SYS_PCIE1_MEM_VIRT + 0x10000000),
		      (CONFIG_SYS_PCIE1_MEM_PHYS + 0x10000000),
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLBe 4:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe280_0000	8M	PCIe IO
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_64M, 1),

#if defined(CONFIG_SYS_RAMBOOT) && defined(CONFIG_SYS_INIT_L2_ADDR)
	/* *I*G - L2SRAM */
	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_L2_ADDR, CONFIG_SYS_INIT_L2_ADDR_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, 5, BOOKE_PAGESZ_256K, 1),
	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_L2_ADDR + 0x40000,
			CONFIG_SYS_INIT_L2_ADDR_PHYS + 0x40000,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, 6, BOOKE_PAGESZ_256K, 1),
#endif
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
