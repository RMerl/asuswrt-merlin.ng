// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>

#define MPFS_SYSREG_SOFT_RESET	((unsigned int *)0x20002088)

int board_init(void)
{
	/* For now nothing to do here. */

	return 0;
}

int board_early_init_f(void)
{
	unsigned int val;

	/* Reset uart peripheral */
	val = readl(MPFS_SYSREG_SOFT_RESET);
	val = (val & ~(1u << 5u));
	writel(val, MPFS_SYSREG_SOFT_RESET);

	return 0;
}
