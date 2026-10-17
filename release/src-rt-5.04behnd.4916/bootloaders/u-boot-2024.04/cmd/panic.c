// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <command.h>

static int do_panic(struct cmd_tbl *cmdtp, int flag, int argc,
		    char * const argv[])
{
	char *text = (argc < 2) ? "" : argv[1];

	panic(text);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	panic,	2,	1,	do_panic,
	"Panic with optional message",
	"[message]"
);
