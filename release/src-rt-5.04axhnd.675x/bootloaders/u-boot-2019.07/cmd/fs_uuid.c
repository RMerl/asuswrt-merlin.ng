// SPDX-License-Identifier: GPL-2.0+
/*
 * cmd_fs_uuid.c -- fsuuid command
 *
 * Copyright (C) 2014, Bachmann electronic GmbH
 */

#include <common.h>
#include <command.h>
#include <fs.h>

static int do_fs_uuid_wrapper(cmd_tbl_t *cmdtp, int flag,
	int argc, char * const argv[])
{
	return do_fs_uuid(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	fsuuid, 4, 1, do_fs_uuid_wrapper,
	"Look up a filesystem UUID",
	"<interface> <dev>:<part>\n"
	"    - print filesystem UUID\n"
	"fsuuid <interface> <dev>:<part> <varname>\n"
	"    - set environment variable to filesystem UUID\n"
);
