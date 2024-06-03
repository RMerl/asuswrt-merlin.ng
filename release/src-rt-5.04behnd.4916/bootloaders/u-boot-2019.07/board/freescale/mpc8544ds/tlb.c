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
	/*
	 * TLB 0:	64M	Non-cacheable, guarded
	 * 0xfc000000	64M	Covers FLASH at 0xFE800000 and 0xFF800000
	 * Out of reset this entry is only 4K.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_BOOT_BLOCK, CONFIG_SYS_BOOT_BLOCK,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_64M, 1),
	/*
	 * TLB 1:	1G	Non-cacheable, guarded
	 * 0x80000000	1G	PCIE  8,9,a,b
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE_VIRT, CONFIG_SYS_PCIE_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_1G, 1),

	/*
	 * TLB 2:	256M	Non-cacheable, guarded
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI_VIRT, CONFIG_SYS_PCI_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 3:	256M	Non-cacheable, guarded
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCI_VIRT + 0x10000000, CONFIG_SYS_PCI_PHYS + 0x10000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),

	/*
	 * TLB 4:	64M	Non-cacheable, guarded
	 * 0xe000_0000	1M	CCSRBAR
	 * 0xe100_0000	255M	PCI IO range
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_64M, 1),

	/*
	 * TLB 5:	64M	Non-cacheable, guarded
	 * 0xf8000000	64M	PIXIS 0xF8000000 - 0xFBFFFFFF
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_LBC_NONCACHE_BASE, CONFIG_SYS_LBC_NONCACHE_BASE,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_64M, 1),
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
