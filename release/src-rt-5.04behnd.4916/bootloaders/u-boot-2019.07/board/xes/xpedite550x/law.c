// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010 Extreme Engineering Solutions, Inc.
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
 *    CCSRBAR and L2-as-SRAM don't need a configured Local Access Window.
 *    If flash is 8M at default position (last 8M), no LAW needed.
 */

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_FLASH_BASE2, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
	SET_LAW(CONFIG_SYS_NAND_BASE, LAW_SIZE_1M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
