// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Keymile AG
 * Rainer Boschung <rainer.boschung@keymile.com>
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <asm/mmu.h>
#include <asm/u-boot.h>

struct fsl_e_tlb_entry tlb_table[] = {
	/* TLB 0 - for temp stack in cache */
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR,
		      CFG_SYS_INIT_RAM_ADDR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 4 * 1024,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 8 * 1024,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CFG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      CFG_SYS_INIT_RAM_ADDR_PHYS + 12 * 1024,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 */
	/* *I*** - Covers boot page */
	SET_TLB_ENTRY(1, 0xfffff000, 0xfffff000,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 1),

	/* *I*G* - CCSRBAR */
	SET_TLB_ENTRY(1, CFG_SYS_CCSRBAR, CFG_SYS_CCSRBAR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 1, BOOKE_PAGESZ_16M, 1),

	/* *I*G* - Flash, localbus */
	/* This will be changed to *I*G* after relocation to RAM. */
	SET_TLB_ENTRY(1, CFG_SYS_FLASH_BASE, CFG_SYS_FLASH_BASE_PHYS,
		      MAS3_SX | MAS3_SR, MAS2_W | MAS2_G,
		      0, 2, BOOKE_PAGESZ_128M, 1),

	/* *I*G* - PCI1 */
	SET_TLB_ENTRY(1, CFG_SYS_PCIE1_MEM_VIRT, CFG_SYS_PCIE1_MEM_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 3, BOOKE_PAGESZ_1G, 1),

	/* *I*G* - PCI1 I/O */
	SET_TLB_ENTRY(1, CFG_SYS_PCIE1_IO_VIRT, CFG_SYS_PCIE1_IO_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 4, BOOKE_PAGESZ_256K, 1),

	/* Bman/Qman */
	SET_TLB_ENTRY(1, CFG_SYS_BMAN_MEM_BASE, CFG_SYS_BMAN_MEM_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 5, BOOKE_PAGESZ_16M, 1),
	SET_TLB_ENTRY(1, CFG_SYS_BMAN_MEM_BASE + 0x01000000,
		      CFG_SYS_BMAN_MEM_PHYS + 0x01000000,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 6, BOOKE_PAGESZ_16M, 1),
	SET_TLB_ENTRY(1, CFG_SYS_QMAN_MEM_BASE, CFG_SYS_QMAN_MEM_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, 0,
		      0, 7, BOOKE_PAGESZ_16M, 1),
	SET_TLB_ENTRY(1, CFG_SYS_QMAN_MEM_BASE + 0x01000000,
		      CFG_SYS_QMAN_MEM_PHYS + 0x01000000,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 8, BOOKE_PAGESZ_16M, 1),

	SET_TLB_ENTRY(1, CFG_SYS_DCSRBAR, CFG_SYS_DCSRBAR_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 9, BOOKE_PAGESZ_4M, 1),

	/* *I*G - NAND */
	SET_TLB_ENTRY(1, CFG_SYS_NAND_BASE, CFG_SYS_NAND_BASE_PHYS,
		      MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 10, BOOKE_PAGESZ_64K, 1),
	/* QRIO */
	SET_TLB_ENTRY(1, CFG_SYS_QRIO_BASE, CFG_SYS_QRIO_BASE_PHYS,
		      MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 11, BOOKE_PAGESZ_64K, 1),
	/* MRAM */
	SET_TLB_ENTRY(1, CFG_SYS_MRAM_BASE, SYS_MRAM_BASE_PHYS,
		      MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 12, BOOKE_PAGESZ_128M, 1),
	/* BFTIC */
	SET_TLB_ENTRY(1, SYS_BFTIC_BASE, SYS_BFTIC_BASE_PHYS,
		      MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 13, BOOKE_PAGESZ_128M, 1),
	/*
	 * entry 14 and 15 has been used hard coded, they will be disabled
	 * in cpu_init_f, so do not use them here!!.
	 */
	/* PAXE */
	SET_TLB_ENTRY(1, CFG_SYS_PAXE_BASE, SYS_PAXE_BASE_PHYS,
		      MAS3_SW | MAS3_SR, MAS2_I | MAS2_G,
		      0, 16, BOOKE_PAGESZ_128M, 1)
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
