// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2023 CS GROUP France
 * Florent TRINH THAI (florent.trinh-thai@csgroup.eu)
 * Stephane FRANJOU (stephane.franjou@csgroup.eu)
 */

#include <config.h>
#include <nand.h>
#include <linux/bitops.h>
#include <linux/mtd/rawnand.h>
#include <asm/io.h>

#define BIT_CLE		BIT(6)
#define BIT_ALE		BIT(5)

static u32 nand_mask(unsigned int ctrl)
{
	return ((ctrl & NAND_CLE) ? BIT_CLE : 0) |
	       ((ctrl & NAND_ALE) ? BIT_ALE : 0);
}

static void nand_hwcontrol(struct mtd_info *mtdinfo, int cmd, unsigned int ctrl)
{
	immap_t __iomem *immr = (immap_t *)CONFIG_SYS_IMMR;
	struct nand_chip *chip = mtd_to_nand(mtdinfo);

	if (ctrl & NAND_CTRL_CHANGE)
		clrsetbits_be32(&immr->qepio.ioport[2].pdat,
				BIT_CLE | BIT_ALE, nand_mask(ctrl));

	if (cmd != NAND_CMD_NONE)
		out_8(chip->IO_ADDR_W, cmd);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->chip_delay	= 60;
	nand->ecc.mode		= NAND_ECC_SOFT;
	nand->cmd_ctrl		= nand_hwcontrol;

	return 0;
}
