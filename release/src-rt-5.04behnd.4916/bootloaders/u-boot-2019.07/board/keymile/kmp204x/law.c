// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 *
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/mmu.h>

struct law_entry law_table[] = {
#ifdef CONFIG_SYS_BMAN_MEM_PHYS
	SET_LAW(CONFIG_SYS_BMAN_MEM_PHYS, LAW_SIZE_2M, LAW_TRGT_IF_BMAN),
#endif
#ifdef CONFIG_SYS_QMAN_MEM_PHYS
	SET_LAW(CONFIG_SYS_QMAN_MEM_PHYS, LAW_SIZE_2M, LAW_TRGT_IF_QMAN),
#endif
#ifdef CONFIG_SYS_DCSRBAR_PHYS
	/* Limit DCSR to 32M to access NPC Trace Buffer */
	SET_LAW(CONFIG_SYS_DCSRBAR_PHYS, LAW_SIZE_32M, LAW_TRGT_IF_DCSR),
#endif
#ifdef CONFIG_SYS_NAND_BASE_PHYS
	SET_LAW(CONFIG_SYS_NAND_BASE_PHYS, LAW_SIZE_32K, LAW_TRGT_IF_LBC),
#endif
	SET_LAW(CONFIG_SYS_QRIO_BASE_PHYS, LAW_SIZE_64K, LAW_TRGT_IF_LBC),
#ifdef CONFIG_SYS_LBAPP1_BASE_PHYS
	SET_LAW(CONFIG_SYS_LBAPP1_BASE_PHYS, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
#endif
#ifdef CONFIG_SYS_LBAPP2_BASE_PHYS
	SET_LAW(CONFIG_SYS_LBAPP2_BASE_PHYS, LAW_SIZE_256M, LAW_TRGT_IF_LBC),
#endif
};

int num_law_entries = ARRAY_SIZE(law_table);
