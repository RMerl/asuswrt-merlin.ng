// SPDX-License-Identifier: GPL-2.0+
/*
 * board/silinux/ek874/ek874.c
 *     This file is ek874 board support.
 *
 * Copyright (C) 2021 Renesas Electronics Corporation
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>

#define RST_BASE	0xE6160000
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_CA53_CODE	0x5A5A000F

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	return 0;
}

void reset_cpu(void)
{
	writel(RST_CA53_CODE, RST_CA53RESCNT);
}
