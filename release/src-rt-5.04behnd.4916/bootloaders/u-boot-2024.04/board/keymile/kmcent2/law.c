// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Keymile AG
 * Rainer Boschung <rainer.boschung@keymile.com>
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <asm/fsl_law.h>

struct law_entry law_table[] = {
	SET_LAW(CFG_SYS_BMAN_MEM_PHYS, LAW_SIZE_32M, LAW_TRGT_IF_BMAN),
	SET_LAW(CFG_SYS_QMAN_MEM_PHYS, LAW_SIZE_32M, LAW_TRGT_IF_QMAN),
	SET_LAW(CFG_SYS_DCSRBAR_PHYS, LAW_SIZE_4M, LAW_TRGT_IF_DCSR),
	SET_LAW(CFG_SYS_FLASH_BASE_PHYS, LAW_SIZE_64M, LAW_TRGT_IF_IFC),
	SET_LAW(CFG_SYS_NAND_BASE_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_IFC),
	SET_LAW(CFG_SYS_QRIO_BASE_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_IFC),
	SET_LAW(SYS_LAWAPP_BASE_PHYS, LAW_SIZE_512M, LAW_TRGT_IF_IFC),
/* other application LAW are not used in u-boot */
};

int num_law_entries = ARRAY_SIZE(law_table);
