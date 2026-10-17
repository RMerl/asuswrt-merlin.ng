// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *     Tom Rini <trini@ti.com>
 *
 * Initial Code from:
 *     Richard Woodruff <r-woodruff2@ti.com>
 *     Jian Zhang <jzhang@ti.com>
 */

#include <common.h>
#include <jffs2/load_kernel.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/omap_gpmc.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>

/*
 * Many boards will want to know the results of the NAND_CMD_READID command
 * in order to decide what to do about DDR initialization.  This function
 * allows us to do that very early and to pass those results back to the
 * board so it can make whatever decisions need to be made.
 */
int identify_nand_chip(int *mfr, int *id)
{
	int loops = 1000;

	/* Make sure that we have setup GPMC for NAND correctly. */
	set_gpmc_cs0(MTD_DEV_TYPE_NAND);

	sdelay(2000);

	/* Issue a RESET and then READID */
	writeb(NAND_CMD_RESET, &gpmc_cfg->cs[0].nand_cmd);
	writeb(NAND_CMD_STATUS, &gpmc_cfg->cs[0].nand_cmd);
	while ((readl(&gpmc_cfg->cs[0].nand_dat) & NAND_STATUS_READY)
	                                        != NAND_STATUS_READY) {
		sdelay(100);
		if (--loops == 0)
			return 1;
	}
	writeb(NAND_CMD_READID, &gpmc_cfg->cs[0].nand_cmd);

	/* Set the address to read to 0x0 */
	writeb(0x0, &gpmc_cfg->cs[0].nand_adr);

	/* Read off the manufacturer and device id. */
	*mfr = readb(&gpmc_cfg->cs[0].nand_dat);
	*id = readb(&gpmc_cfg->cs[0].nand_dat);

	return 0;
}
