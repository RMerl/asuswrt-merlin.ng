// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

/*
 * LAW (Local Access Window) configuration:
 *
 * 0x0000_0000	DDR			256M
 * 0x1000_0000	DDR2			256M
 * 0x8000_0000	PCIE1 MEM		512M
 * 0xa000_0000	PCIE2 MEM		512M
 * 0xc000_0000	RapidIO			512M
 * 0xe200_0000	PCIE1 IO		16M
 * 0xe300_0000	PCIE2 IO		16M
 * 0xf800_0000	CCSRBAR			2M
 * 0xfe00_0000	FLASH (boot bank)	32M
 *
 */


struct law_entry law_table[] = {
#if !defined(CONFIG_SPD_EEPROM)
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_DDR_1),
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE + 0x10000000,
		 LAW_SIZE_256M, LAW_TRGT_IF_DDR_2),
#endif
	SET_LAW(0xf8000000, LAW_SIZE_2M, LAW_TRGT_IF_LBC),
	SET_LAW(0xfe000000, LAW_SIZE_32M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
