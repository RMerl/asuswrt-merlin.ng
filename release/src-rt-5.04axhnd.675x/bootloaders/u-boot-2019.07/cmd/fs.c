// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
 *
 * Inspired by cmd_ext_common.c, cmd_fat.c.
 */

#include <common.h>
#include <command.h>
#include <fs.h>

static int do_size_wrapper(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return do_size(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	size,	4,	0,	do_size_wrapper,
	"determine a file's size",
	"<interface> <dev[:part]> <filename>\n"
	"    - Find file 'filename' from 'dev' on 'interface'\n"
	"      and determine its size."
);

static int do_load_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	load,	7,	0,	do_load_wrapper,
	"load binary file from a filesystem",
	"<interface> [<dev[:part]> [<addr> [<filename> [bytes [pos]]]]]\n"
	"    - Load binary file 'filename' from partition 'part' on device\n"
	"       type 'interface' instance 'dev' to address 'addr' in memory.\n"
	"      'bytes' gives the size to load in bytes.\n"
	"      If 'bytes' is 0 or omitted, the file is read until the end.\n"
	"      'pos' gives the file byte position to start reading from.\n"
	"      If 'pos' is 0 or omitted, the file is read from the start."
)

static int do_save_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	return do_save(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	save,	7,	0,	do_save_wrapper,
	"save file to a filesystem",
	"<interface> <dev[:part]> <addr> <filename> bytes [pos]\n"
	"    - Save binary file 'filename' to partition 'part' on device\n"
	"      type 'interface' instance 'dev' from addr 'addr' in memory.\n"
	"      'bytes' gives the size to save in bytes and is mandatory.\n"
	"      'pos' gives the file byte position to start writing to.\n"
	"      If 'pos' is 0 or omitted, the file is written from the start."
)

static int do_ls_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	ls,	4,	1,	do_ls_wrapper,
	"list files in a directory (default /)",
	"<interface> [<dev[:part]> [directory]]\n"
	"    - List files in directory 'directory' of partition 'part' on\n"
	"      device type 'interface' instance 'dev'."
)

static int do_ln_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	return do_ln(cmdtp, flag, argc, argv, FS_TYPE_ANY);
}

U_BOOT_CMD(
	ln,	5,	1,	do_ln_wrapper,
	"Create a symbolic link",
	"<interface> <dev[:part]> target linkname\n"
	"    - create a symbolic link to 'target' with the name 'linkname' on\n"
	"      device type 'interface' instance 'dev'."
)

static int do_fstype_wrapper(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	return do_fs_type(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	fstype, 4, 1, do_fstype_wrapper,
	"Look up a filesystem type",
	"<interface> <dev>:<part>\n"
	"- print filesystem type\n"
	"fstype <interface> <dev>:<part> <varname>\n"
	"- set environment variable to filesystem type\n"
);
