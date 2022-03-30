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
 * Notes:
 *    CCSRBAR don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_FLASH_BASE2, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
#ifdef CONFIG_SYS_NAND_BASE
	/* NAND LAW covers 2 NAND flashes */
	SET_LAW(CONFIG_SYS_NAND_BASE, LAW_SIZE_512K, LAW_TRGT_IF_LBC),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
