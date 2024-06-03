// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2009 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>

#include "bcsr.h"

void enable_8569mds_flash_write(void)
{
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 17), BCSR17_FLASH_nWP);
}

void disable_8569mds_flash_write(void)
{
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 17), BCSR17_FLASH_nWP);
}

void enable_8569mds_qe_uec(void)
{
#if defined(CONFIG_SYS_UCC_RGMII_MODE)
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 7),
			BCSR7_UCC1_GETH_EN | BCSR7_UCC1_RGMII_EN);
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 8),
			BCSR8_UCC2_GETH_EN | BCSR8_UCC2_RGMII_EN);
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 9),
			BCSR9_UCC3_GETH_EN | BCSR9_UCC3_RGMII_EN);
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 10),
			BCSR10_UCC4_GETH_EN | BCSR10_UCC4_RGMII_EN);
#elif defined(CONFIG_SYS_UCC_RMII_MODE)
	/* Set UCC1-4 working at RMII mode */
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 7),
			BCSR7_UCC1_GETH_EN | BCSR7_UCC1_RGMII_EN);
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 8),
			BCSR8_UCC2_GETH_EN | BCSR8_UCC2_RGMII_EN);
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 9),
			BCSR9_UCC3_GETH_EN | BCSR9_UCC3_RGMII_EN);
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 10),
			BCSR10_UCC4_GETH_EN | BCSR10_UCC4_RGMII_EN);
	setbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 9), BCSR9_UCC3_RMII_EN);
#endif
}

void disable_8569mds_brd_eeprom_write_protect(void)
{
	clrbits_8((u8 *)(CONFIG_SYS_BCSR_BASE + 7), BCSR7_BRD_WRT_PROTECT);
}
