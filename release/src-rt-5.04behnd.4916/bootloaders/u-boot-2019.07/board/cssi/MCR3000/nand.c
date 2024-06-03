// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2017 CS Systemes d'Information
 * Florent Trinh Thai <florent.trinh-thai@c-s.fr>
 * Christophe Leroy <christophe.leroy@c-s.fr>
 */

#include <config.h>
#include <common.h>
#include <nand.h>
#include <asm/io.h>

#define BIT_CLE			((unsigned short)0x0800)
#define BIT_ALE			((unsigned short)0x0400)
#define BIT_NCE			((unsigned short)0x1000)

static void nand_hwcontrol(struct mtd_info *mtdinfo, int cmd, unsigned int ctrl)
{
	struct nand_chip *this	= mtd_to_nand(mtdinfo);
	immap_t __iomem *immr	= (immap_t __iomem *)CONFIG_SYS_IMMR;
	unsigned short pddat	= 0;

	/* The hardware control change */
	if (ctrl & NAND_CTRL_CHANGE) {
		pddat = in_be16(&immr->im_ioport.iop_pddat);

		/* Clearing ALE and CLE */
		pddat &= ~(BIT_CLE | BIT_ALE);

		/* Driving NCE pin */
		if (ctrl & NAND_NCE)
			pddat &= ~BIT_NCE;
		else
			pddat |= BIT_NCE;

		/* Driving CLE and ALE pin */
		if (ctrl & NAND_CLE)
			pddat |= BIT_CLE;
		if (ctrl & NAND_ALE)
			pddat |= BIT_ALE;

		out_be16(&immr->im_ioport.iop_pddat, pddat);
	}

	/* Writing the command */
	if (cmd != NAND_CMD_NONE)
		out_8(this->IO_ADDR_W, cmd);
}

int board_nand_init(struct nand_chip *nand)
{
	immap_t __iomem *immr	= (immap_t __iomem *)CONFIG_SYS_IMMR;

	/* Set GPIO Port */
	setbits_be16(&immr->im_ioport.iop_pddir, 0x1c00);
	clrbits_be16(&immr->im_ioport.iop_pdpar, 0x1c00);
	clrsetbits_be16(&immr->im_ioport.iop_pddat, 0x0c00, 0x1000);

	nand->chip_delay	= 60;
	nand->ecc.mode		= NAND_ECC_SOFT;
	nand->cmd_ctrl		= nand_hwcontrol;

	return 0;
}
