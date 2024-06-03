// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/mmu.h>

struct fsl_e_tlb_entry tlb_table[] = {
	/* TLB 0 - for temp stack in cache */
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS,
		      MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 4 * 1024,
		      MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 8 * 1024,
		      MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 12 * 1024,
		      MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
#ifdef CPLD_BASE
	SET_TLB_ENTRY(0, CPLD_BASE, CPLD_BASE_PHYS,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 0),
#endif

#ifdef PIXIS_BASE
	SET_TLB_ENTRY(0, PIXIS_BASE, PIXIS_BASE_PHYS,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 0),
#endif

	/* TLB 1 */
	/* *I*** - Covers boot page */
#if defined(CONFIG_SYS_RAMBOOT) && defined(CONFIG_SYS_INIT_L3_ADDR)

#if !defined(CONFIG_SECURE_BOOT)
	/*
	 * *I*G - L3SRAM. When L3 is used as 1M SRAM, the address of the
	 * SRAM is at 0xfff00000, it covered the 0xfffff000.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_L3_ADDR, CONFIG_SYS_INIT_L3_ADDR,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, 0, BOOKE_PAGESZ_1M, 1),
#else
	/*
	 * *I*G - L3SRAM. When L3 is used as 1M SRAM, in case of Secure Boot
	 * the physical address of the SRAM is at CONFIG_SYS_INIT_L3_ADDR,
	 * and virtual address is CONFIG_SYS_MONITOR_BASE
	 */

	SET_TLB_ENTRY(1, CONFIG_SYS_MONITOR_BASE & 0xfff00000,
			CONFIG_SYS_INIT_L3_ADDR & 0xfff00000,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, 0, BOOKE_PAGESZ_1M, 1),
#endif

#elif defined(CONFIG_SRIO_PCIE_BOOT_SLAVE)
	/*
	 * SRIO_PCIE_BOOT-SLAVE. When slave boot, the address of the
	 * space is at 0xfff00000, it covered the 0xfffff000.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR,
			CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_W|MAS2_G,
			0, 0, BOOKE_PAGESZ_1M, 1),
#else
	SET_TLB_ENTRY(1, 0xfffff000, 0xfffff000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 1),
#endif

	/* *I*G* - CCSRBAR */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_16M, 1),

	/* *I*G* - Flash, localbus */
	/* This will be changed to *I*G* after relocation to RAM. */
	SET_TLB_ENTRY(1, CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE_PHYS,
		      MAS3_SX|MAS3_SR, MAS2_W|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/* *I*G* - PCI */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT, CONFIG_SYS_PCIE1_MEM_PHYS,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_1G, 1),

	/* *I*G* - PCI */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT + 0x40000000,
		      CONFIG_SYS_PCIE1_MEM_PHYS + 0x40000000,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_256M, 1),

	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT + 0x50000000,
		      CONFIG_SYS_PCIE1_MEM_PHYS + 0x50000000,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_256M, 1),

	/* *I*G* - PCI I/O */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_IO_VIRT, CONFIG_SYS_PCIE1_IO_PHYS,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 6, BOOKE_PAGESZ_256K, 1),

	/* Bman/Qman */
#ifdef CONFIG_SYS_BMAN_MEM_PHYS
	SET_TLB_ENTRY(1, CONFIG_SYS_BMAN_MEM_BASE, CONFIG_SYS_BMAN_MEM_PHYS,
		      MAS3_SW|MAS3_SR, 0,
		      0, 9, BOOKE_PAGESZ_1M, 1),
	SET_TLB_ENTRY(1, CONFIG_SYS_BMAN_MEM_BASE + 0x00100000,
		      CONFIG_SYS_BMAN_MEM_PHYS + 0x00100000,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 10, BOOKE_PAGESZ_1M, 1),
#endif
#ifdef CONFIG_SYS_QMAN_MEM_PHYS
	SET_TLB_ENTRY(1, CONFIG_SYS_QMAN_MEM_BASE, CONFIG_SYS_QMAN_MEM_PHYS,
		      MAS3_SW|MAS3_SR, 0,
		      0, 11, BOOKE_PAGESZ_1M, 1),
	SET_TLB_ENTRY(1, CONFIG_SYS_QMAN_MEM_BASE + 0x00100000,
		      CONFIG_SYS_QMAN_MEM_PHYS + 0x00100000,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 12, BOOKE_PAGESZ_1M, 1),
#endif
#ifdef CONFIG_SYS_DCSRBAR_PHYS
	SET_TLB_ENTRY(1, CONFIG_SYS_DCSRBAR, CONFIG_SYS_DCSRBAR_PHYS,
		      MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 13, BOOKE_PAGESZ_4M, 1),
#endif
#ifdef CONFIG_SYS_NAND_BASE
	/*
	 * *I*G - NAND
	 * entry 14 and 15 has been used hard coded, they will be disabled
	 * in cpu_init_f, so we use entry 16 for nand.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_NAND_BASE, CONFIG_SYS_NAND_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, 16, BOOKE_PAGESZ_1M, 1),
#endif
#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
	/*
	 * SRIO_PCIE_BOOT-SLAVE. 1M space from 0xffe00000 for
	 * fetching ucode and ENV from master
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR,
		CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS,
		MAS3_SX|MAS3_SW|MAS3_SR, MAS2_G,
		0, 17, BOOKE_PAGESZ_1M, 1),
#endif
};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
