// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
	SET_LAW(CONFIG_SYS_FLASH_BASE_PHYS, LAW_SIZE_32M, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_CPLD_BASE_PHYS, LAW_SIZE_128K, LAW_TRGT_IF_IFC),
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_1M, LAW_TRGT_IF_IFC),
};

int num_law_entries = ARRAY_SIZE(law_table);
