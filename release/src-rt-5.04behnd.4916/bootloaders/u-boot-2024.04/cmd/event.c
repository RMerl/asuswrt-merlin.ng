// SPDX-License-Identifier: GPL-2.0+
/*
 * Command-line access to events
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <event.h>

static int do_event_list(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	event_show_spy_list();

	return 0;
}

U_BOOT_LONGHELP(event,
	"list - list event spies");

U_BOOT_CMD_WITH_SUBCMDS(event, "Events", event_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_event_list));
