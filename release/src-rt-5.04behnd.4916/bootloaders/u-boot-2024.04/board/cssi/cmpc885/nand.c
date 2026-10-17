// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2020 CS Group
 * Florent Trinh Thai <florent.trinh-thai@c-s.fr>
 * Christophe Leroy <christophe.leroy@c-s.fr>
 * Charles Frey <charles.frey@c-s.fr>
 */

#include <config.h>
#include <nand.h>
#include <linux/bitops.h>
#include <linux/mtd/rawnand.h>
#include <asm/io.h>

#define BIT_CLE		BIT(3)
#define BIT_ALE		BIT(2)
#define BIT_NCE		BIT(0)

static u32 nand_mask(unsigned int ctrl)
{
	return ((ctrl & NAND_CLE) ? BIT_CLE : 0) |
	       ((ctrl & NAND_ALE) ? BIT_ALE : 0) |
	       (!(ctrl & NAND_NCE) ? BIT_NCE : 0);
}

static void nand_hwcontrol(struct mtd_info *mtdinfo, int cmd, unsigned int ctrl)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	struct nand_chip *chip = mtd_to_nand(mtdinfo);

	if (ctrl & NAND_CTRL_CHANGE)
		clrsetbits_be16(&immr->im_ioport.iop_pddat,
				BIT_CLE | BIT_ALE | BIT_NCE, nand_mask(ctrl));

	if (cmd != NAND_CMD_NONE)
		out_8(chip->IO_ADDR_W, cmd);
}

int board_nand_init(struct nand_chip *chip)
{
	chip->chip_delay	= 60;
	chip->ecc.mode		= NAND_ECC_SOFT;
	chip->cmd_ctrl		= nand_hwcontrol;

	return 0;
}
