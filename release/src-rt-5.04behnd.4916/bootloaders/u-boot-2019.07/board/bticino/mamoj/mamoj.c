// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Simone CIANNI <simone.cianni@bticino.it>
 * Copyright (C) 2018 Raffaele RECALCATI <raffaele.recalcati@bticino.it>
 * Copyright (C) 2018 Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}
