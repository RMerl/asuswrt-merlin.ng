// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2007 Freescale Semiconductor.
 */

#include <common.h>
#include <asm/io.h>

#include "bcsr.h"

void enable_8568mds_duart(void)
{
	volatile uint* duart_mux	= (uint *)(CONFIG_SYS_CCSRBAR + 0xe0060);
	volatile uint* devices		= (uint *)(CONFIG_SYS_CCSRBAR + 0xe0070);
	volatile u8 *bcsr		= (u8 *)(CONFIG_SYS_BCSR);

	*duart_mux = 0x80000000;	/* Set the mux to Duart on PMUXCR */
	*devices  = 0;			/* Enable all peripheral devices */
	bcsr[5] |= 0x01;		/* Enable Duart in BCSR*/
}

void enable_8568mds_flash_write(void)
{
	volatile u8 *bcsr = (u8 *)(CONFIG_SYS_BCSR);

	bcsr[9] |= 0x01;
}

void disable_8568mds_flash_write(void)
{
	volatile u8 *bcsr = (u8 *)(CONFIG_SYS_BCSR);

	bcsr[9] &= ~(0x01);
}

void enable_8568mds_qe_mdio(void)
{
	u8 *bcsr = (u8 *)(CONFIG_SYS_BCSR);

	bcsr[7] |= 0x01;
}

#if defined(CONFIG_UEC_ETH1) || defined(CONFIG_UEC_ETH2)
void reset_8568mds_uccs(void)
{
	volatile u8 *bcsr = (u8 *)(CONFIG_SYS_BCSR);

	/* Turn off UCC1 & UCC2 */
	out_8(&bcsr[8], in_8(&bcsr[8]) & ~BCSR_UCC1_GETH_EN);
	out_8(&bcsr[9], in_8(&bcsr[9]) & ~BCSR_UCC2_GETH_EN);

	/* Mode is RGMII, all bits clear */
	out_8(&bcsr[11], in_8(&bcsr[11]) & ~(BCSR_UCC1_MODE_MSK |
					     BCSR_UCC2_MODE_MSK));

	/* Turn UCC1 & UCC2 on */
	out_8(&bcsr[8], in_8(&bcsr[8]) | BCSR_UCC1_GETH_EN);
	out_8(&bcsr[9], in_8(&bcsr[9]) | BCSR_UCC2_GETH_EN);
}
#endif
