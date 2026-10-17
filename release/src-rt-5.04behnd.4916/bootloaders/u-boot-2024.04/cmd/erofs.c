// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Huang Jianan <jnhuang95@gmail.com>
 *
 * Author: Huang Jianan <jnhuang95@gmail.com>
 *
 * erofs.c:	implements EROFS related commands
 */

#include <command.h>
#include <fs.h>
#include <erofs.h>

static int do_erofs_ls(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_EROFS);
}

U_BOOT_CMD(erofsls, 4, 1, do_erofs_ls,
	   "List files in directory. Default: root (/).",
	   "<interface> [<dev[:part]>] [directory]\n"
	   "    - list files from 'dev' on 'interface' in 'directory'\n"
);

static int do_erofs_load(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_EROFS);
}

U_BOOT_CMD(erofsload, 7, 0, do_erofs_load,
	   "load binary file from a EROFS filesystem",
	   "<interface> [<dev[:part]> [<addr> [<filename> [bytes [pos]]]]]\n"
	   "    - Load binary file 'filename' from 'dev' on 'interface'\n"
	   "      to address 'addr' from EROFS filesystem.\n"
	   "      'pos' gives the file position to start loading from.\n"
	   "      If 'pos' is omitted, 0 is used. 'pos' requires 'bytes'.\n"
	   "      'bytes' gives the size to load. If 'bytes' is 0 or omitted,\n"
	   "      the load stops on end of file.\n"
	   "      If either 'pos' or 'bytes' are not aligned to\n"
	   "      ARCH_DMA_MINALIGN then a misaligned buffer warning will\n"
	   "      be printed and performance will suffer for the load."
);
