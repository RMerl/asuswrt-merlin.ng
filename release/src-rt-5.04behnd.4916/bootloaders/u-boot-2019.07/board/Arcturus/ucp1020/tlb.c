// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013-2015 Arcturus Networks, Inc
 *           http://www.arcturusnetworks.com/products/ucp1020/
 * based on board/freescale/p1_p2_rdb_pc/tlb.c
 * original copyright follows:
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/mmu.h>

struct fsl_e_tlb_entry tlb_table[] = {
	/* TLB 0 - for temp stack in cache */
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 4 * 1024,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 8 * 1024,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 12 * 1024,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 */
	/* *I*** - Covers boot page */
	SET_TLB_ENTRY(1, 0xfffff000, 0xfffff000,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I,
		      0, 0, BOOKE_PAGESZ_4K, 1),

	/* *I*G* - CCSRBAR */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 1, BOOKE_PAGESZ_1M, 1),

#ifndef CONFIG_SPL_BUILD
	/* W**G* - Flash/promjet, localbus */
	/* This will be changed to *I*G* after relocation to RAM. */
	SET_TLB_ENTRY(1, CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE_PHYS,
		      MAS3_SX | MAS3_SR, MAS2_W | MAS2_G,
		      0, 2, BOOKE_PAGESZ_64M, 1),

#ifdef CONFIG_PCI
	/* *I*G* - PCI memory 1.5G */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT, CONFIG_SYS_PCIE1_MEM_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 3, BOOKE_PAGESZ_1G, 1),

	/* *I*G* - PCI I/O effective: 192K  */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_IO_VIRT, CONFIG_SYS_PCIE1_IO_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 4, BOOKE_PAGESZ_256K, 1),
#endif

#ifdef CONFIG_VSC7385_ENET
	/* *I*G - VSC7385 Switch */
	SET_TLB_ENTRY(1, CONFIG_SYS_VSC7385_BASE, CONFIG_SYS_VSC7385_BASE_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 5, BOOKE_PAGESZ_1M, 1),
#endif
#endif /* not SPL */

#ifdef CONFIG_SYS_NAND_BASE
	/* *I*G - NAND */
	SET_TLB_ENTRY(1, CONFIG_SYS_NAND_BASE, CONFIG_SYS_NAND_BASE_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 7, BOOKE_PAGESZ_1M, 1),
#endif

#if defined(CONFIG_SYS_RAMBOOT) || \
	(defined(CONFIG_SPL) && !defined(CONFIG_SPL_COMMON_INIT_DDR))
	/* *I*G - eSDHC/eSPI/NAND boot */
	SET_TLB_ENTRY(1, CONFIG_SYS_DDR_SDRAM_BASE, CONFIG_SYS_DDR_SDRAM_BASE,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_M,
		      0, 8, BOOKE_PAGESZ_1G, 1),

#endif /* RAMBOOT/SPL */

#ifdef CONFIG_SYS_INIT_L2_ADDR
	/* *I*G - L2SRAM */
	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_L2_ADDR, CONFIG_SYS_INIT_L2_ADDR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_G,
		      0, 11, BOOKE_PAGESZ_256K, 1),
#if CONFIG_SYS_L2_SIZE >= (256 << 10)
	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_L2_ADDR + 0x40000,
		      CONFIG_SYS_INIT_L2_ADDR_PHYS + 0x40000,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 12, BOOKE_PAGESZ_256K, 1)
#endif
#endif
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
