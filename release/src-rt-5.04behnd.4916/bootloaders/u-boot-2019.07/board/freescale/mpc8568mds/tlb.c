// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
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
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024 , CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024 , CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024 , CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 Initializations */
	/*
	 * TLBe 0:	16M	Non-cacheable, guarded
	 * 0xff000000	16M	FLASH (upper half)
	 * Out of reset this entry is only 4K.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_FLASH_BASE + 0x1000000, CONFIG_SYS_FLASH_BASE + 0x1000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_16M, 1),

	/*
	 * TLBe 1:	16M	Non-cacheable, guarded
	 * 0xfe000000	16M	FLASH (lower half)
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_16M, 1),

	/*
	 * TLBe 2:	1G	Non-cacheable, guarded
	 * 0x80000000	512M	PCI1 MEM
	 * 0xa0000000	512M	PCIe MEM
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI1_MEM_VIRT, CONFIG_SYS_PCI1_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_1G, 1),

	/*
	 * TLBe 3:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe200_0000	8M	PCI1 IO
	 * 0xe280_0000	8M	PCIe IO
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLBe 4:	64M	Cacheable, non-guarded
	 * 0xf000_0000	64M	LBC SDRAM
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_LBC_SDRAM_BASE, CONFIG_SYS_LBC_SDRAM_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_M,
		      0, 4, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLBe 5:	256K	Non-cacheable, guarded
	 * 0xf8000000	32K BCSR
	 * 0xf8008000	32K PIB (CS4)
	 * 0xf8010000	32K PIB (CS5)
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_BCSR_BASE, CONFIG_SYS_BCSR_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_256K, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
