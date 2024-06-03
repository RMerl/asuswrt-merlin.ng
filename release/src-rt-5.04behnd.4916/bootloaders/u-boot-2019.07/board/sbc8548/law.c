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
 * LAW(Local Access Window) configuration:
 *
 * 0x0000_0000	0x0fff_ffff	DDR			256M
 * 0x8000_0000	0x9fff_ffff	PCI1 MEM		512M
 * 0xa000_0000	0xbfff_ffff	PCIe MEM		512M
 * 0xe000_0000	0xe000_ffff	CCSR			1M
 * 0xe200_0000	0xe27f_ffff	PCI1 IO			8M
 * 0xe280_0000	0xe2ff_ffff	PCIe IO			8M
 * 0xec00_0000	0xefff_ffff	FLASH (2nd bank)	64M
 * 0xf000_0000	0xf7ff_ffff	SDRAM			128M
 * 0xf8b0_0000	0xf80f_ffff	EEPROM			1M
 * 0xff80_0000	0xffff_ffff	FLASH (boot bank)	8M
 *
 * If swapped CS0/CS6 via JP12+SW2.8:
 * 0xef80_0000	0xefff_ffff	FLASH (2nd bank)	8M
 * 0xfc00_0000	0xffff_ffff	FLASH (boot bank)	64M
 *
 * Notes:
 *	CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *	If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
#ifdef CONFIG_SYS_ALT_BOOT
	SET_LAW(CONFIG_SYS_ALT_FLASH, LAW_SIZE_8M, LAW_TRGT_IF_LBC),
#else
	SET_LAW(CONFIG_SYS_ALT_FLASH, LAW_SIZE_64M, LAW_TRGT_IF_LBC),
#endif
#ifndef CONFIG_SPD_EEPROM
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_DDR),
#endif
#ifdef CONFIG_SYS_LBC_SDRAM_BASE
	/* LBC window - maps 256M 0xf0000000 -> 0xffffffff */
	SET_LAW(CONFIG_SYS_LBC_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
#else
	/* LBC window - maps 128M 0xf8000000 -> 0xffffffff */
	SET_LAW(CONFIG_SYS_EPLD_BASE, LAW_SIZE_128M, LAW_TRGT_IF_LBC),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
