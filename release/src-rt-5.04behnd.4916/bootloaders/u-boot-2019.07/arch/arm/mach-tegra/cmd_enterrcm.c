// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012-2019, NVIDIA CORPORATION. All rights reserved.
 *
 * Derived from code (arch/arm/lib/reset.c) that is:
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * (C) Copyright 2004 Texas Insturments
 */

#include <common.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/pmc.h>

static int do_enterrcm(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	puts("Entering RCM...\n");
	udelay(50000);

	tegra_pmc_writel(2, PMC_SCRATCH0);
	disable_interrupts();
	reset_cpu(0);

	return 0;
}

U_BOOT_CMD(
	enterrcm, 1, 0, do_enterrcm,
	"reset Tegra and enter USB Recovery Mode",
	""
);
