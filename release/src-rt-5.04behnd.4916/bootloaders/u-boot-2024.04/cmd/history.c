// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <cli.h>

static int do_history(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	cread_print_hist_list();

	return 0;
}

U_BOOT_CMD(
	history,	CONFIG_SYS_MAXARGS,	1,	do_history,
	"print command history",
	""
);
