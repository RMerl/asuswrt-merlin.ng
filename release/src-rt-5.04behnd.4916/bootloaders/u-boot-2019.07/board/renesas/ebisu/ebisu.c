// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/ebisu/ebisu.c
 *     This file is Ebisu board support.
 *
 * Copyright (C) 2018 Marek Vasut <marek.vasut+renesas@gmail.com>
 */

#include <common.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void)
{
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CA53RESCNT	(RST_BASE + 0x44)
#define RST_RSTOUTCR	(RST_BASE + 0x58)
#define RST_CA57_CODE	0xA5A5000F
#define RST_CA53_CODE	0x5A5A000F

void reset_cpu(ulong addr)
{
	unsigned long midr, cputype;

	asm volatile("mrs %0, midr_el1" : "=r" (midr));
	cputype = (midr >> 4) & 0xfff;

	if (cputype == 0xd03)
		writel(RST_CA53_CODE, RST_CA53RESCNT);
	else if (cputype == 0xd07)
		writel(RST_CA57_CODE, RST_CA57RESCNT);
	else
		hang();
}
