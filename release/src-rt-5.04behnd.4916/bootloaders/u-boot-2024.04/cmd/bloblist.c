// SPDX-License-Identifier: GPL-2.0+
/*
 * Command-line access to bloblist features
 *
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bloblist.h>
#include <command.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int do_bloblist_info(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	bloblist_show_stats();

	return 0;
}

static int do_bloblist_list(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	bloblist_show_list();

	return 0;
}

U_BOOT_LONGHELP(bloblist,
	"info   - show information about the bloblist\n"
	"bloblist list   - list blobs in the bloblist");

U_BOOT_CMD_WITH_SUBCMDS(bloblist, "Bloblists", bloblist_help_text,
	U_BOOT_SUBCMD_MKENT(info, 1, 1, do_bloblist_info),
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_bloblist_list));
