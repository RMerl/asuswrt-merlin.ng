// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#include <common.h>
#include <miiphy.h>
#include <asm/arch/orion5x.h>
#include "../common/common.h"
#include <spl.h>
#include <ns16550.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* arch number of board */
	gd->bd->bi_arch_number = MACH_TYPE_EDMINI_V2;

	/* boot parameter start at 256th byte of RAM base */
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	return 0;
}

#if defined(CONFIG_CMD_NET) && defined(CONFIG_RESET_PHY_R)
/* Configure and enable MV88E1116 PHY */
void reset_phy(void)
{
	mv_phy_88e1116_init("egiga0", 8);
}
#endif /* CONFIG_RESET_PHY_R */

/*
 * SPL serial setup and NOR boot device selection
 */

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NOR;
}

#endif /* CONFIG_SPL_BUILD */
