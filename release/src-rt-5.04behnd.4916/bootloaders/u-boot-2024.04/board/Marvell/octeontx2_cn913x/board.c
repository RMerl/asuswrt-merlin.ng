// SPDX-License-Identifier:    GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 * Copyright (C) 2020 Marvell International Ltd.
 */

#include <dm.h>
#include <power/regulator.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int soc_early_init_f(void)
{
	return 0;
}

int board_early_init_f(void)
{
	soc_early_init_f();

	return 0;
}

int board_early_init_r(void)
{
	if (CONFIG_IS_ENABLED(DM_REGULATOR)) {
		/* Check if any existing regulator should be turned down */
		regulators_enable_boot_off(false);
	}

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_late_init(void)
{
	return 0;
}
