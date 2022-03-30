// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008,2010-2011 Freescale Semiconductor, Inc.
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
 * 0x0000_0000     0x7fff_ffff     DDR                     2G
 * if PCI (prepend 0xc_0000_0000 if CONFIG_PHYS_64BIT)
 * 0x8000_0000     0x9fff_ffff     PCIE1 MEM                512M
 * 0xa000_0000     0xbfff_ffff     PCIE2 MEM                512M
 * else if RIO (prepend 0xc_0000_0000 if CONFIG_PHYS_64BIT)
 * 0x8000_0000     0x9fff_ffff     RapidIO                 512M
 * endif
 * (prepend 0xf_0000_0000 if CONFIG_PHYS_64BIT)
 * 0xffc0_0000     0xffc0_ffff     PCIE1 IO                 64K
 * 0xffc1_0000     0xffc1_ffff     PCIE2 IO                 64K
 * 0xffe0_0000     0xffef_ffff     CCSRBAR                 1M
 * 0xffdf_0000     0xffe0_0000     PIXIS, CF               64K
 * 0xef80_0000     0xefff_ffff     FLASH (boot bank)       8M
 *
 * Notes:
 *    CCSRBAR doesn't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
#if !defined(CONFIG_SPD_EEPROM)
	SET_LAW(CONFIG_SYS_DDR_SDRAM_BASE, LAW_SIZE_256M, LAW_TRGT_IF_DDR_1),
#endif
	SET_LAW(PIXIS_BASE_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_FLASH_BASE_PHYS, LAW_SIZE_8M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
