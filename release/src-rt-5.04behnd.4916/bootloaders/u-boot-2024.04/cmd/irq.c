// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <irq_func.h>

static int do_interrupts(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{

	if (argc != 2)
		return CMD_RET_USAGE;

	/* on */
	if (strncmp(argv[1], "on", 2) == 0)
		enable_interrupts();
	else
		disable_interrupts();

	return 0;
}

U_BOOT_CMD(
	interrupts, 5, 0, do_interrupts,
	"enable or disable interrupts",
	"[on, off]"
);

/* Implemented in $(CPU)/interrupts.c */
int do_irqinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

U_BOOT_CMD(
	irqinfo,    1,    1,     do_irqinfo,
	"print information about IRQs",
	""
);
