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

struct law_entry law_table[] = {
	/* LBC window - maps 256M */
	SET_LAW(CONFIG_SYS_LBC_SDRAM_BASE_PHYS, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
};

int num_law_entries = ARRAY_SIZE(law_table);
