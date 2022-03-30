// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/omap_mmc.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <jffs2/load_kernel.h>
#include <linux/mtd/rawnand.h>
#include "igep00x0.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_DEFAULT();
}

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	int loops = 100;

	/* find out flash memory type, assume NAND first */
	gpmc_cs0_flash = MTD_DEV_TYPE_NAND;
	gpmc_init();

	/* Issue a RESET and then READID */
	writeb(NAND_CMD_RESET, &gpmc_cfg->cs[0].nand_cmd);
	writeb(NAND_CMD_STATUS, &gpmc_cfg->cs[0].nand_cmd);
	while ((readl(&gpmc_cfg->cs[0].nand_dat) & NAND_STATUS_READY)
	                                        != NAND_STATUS_READY) {
		udelay(1);
		if (--loops == 0) {
			gpmc_cs0_flash = MTD_DEV_TYPE_ONENAND;
			gpmc_init();	/* reinitialize for OneNAND */
			break;
		}
	}

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}

void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif
