// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 - 2012 Samsung Electronics
 * EXT4 filesystem implementation in Uboot by
 * Uma Shankar <uma.shankar@samsung.com>
 * Manjunatha C Achar <a.manjunatha@samsung.com>
 *
 * Ext4fs support
 * made from existing cmd_ext2.c file of Uboot
 *
 * (C) Copyright 2004
 * esd gmbh <www.esd-electronics.com>
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 *
 * made from cmd_reiserfs by
 *
 * (C) Copyright 2003 - 2004
 * Sysgo Real-Time Solutions, AG <www.elinos.com>
 * Pavel Bartusek <pba@sysgo.com>
 */

/*
 * Changelog:
 *	0.1 - Newly created file for ext4fs support. Taken from cmd_ext2.c
 *	        file in uboot. Added ext4fs ls load and write support.
 */

#include <common.h>
#include <part.h>
#include <config.h>
#include <command.h>
#include <image.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
#include <ext4fs.h>
#include <linux/stat.h>
#include <malloc.h>
#include <fs.h>

#if defined(CONFIG_CMD_USB) && defined(CONFIG_USB_STORAGE)
#include <usb.h>
#endif

int do_ext4_size(cmd_tbl_t *cmdtp, int flag, int argc,
						char *const argv[])
{
	return do_size(cmdtp, flag, argc, argv, FS_TYPE_EXT);
}

int do_ext4_load(cmd_tbl_t *cmdtp, int flag, int argc,
						char *const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_EXT);
}

int do_ext4_ls(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_EXT);
}

#if defined(CONFIG_CMD_EXT4_WRITE)
int do_ext4_write(cmd_tbl_t *cmdtp, int flag, int argc,
		  char *const argv[])
{
	return do_save(cmdtp, flag, argc, argv, FS_TYPE_EXT);
}

U_BOOT_CMD(ext4write, 7, 1, do_ext4_write,
	   "create a file in the root directory",
	   "<interface> <dev[:part]> <addr> <absolute filename path>\n"
	   "    [sizebytes] [file offset]\n"
	   "    - create a file in / directory");

#endif

U_BOOT_CMD(
	ext4size,	4,	0,	do_ext4_size,
	"determine a file's size",
	"<interface> <dev[:part]> <filename>\n"
	"    - Find file 'filename' from 'dev' on 'interface'\n"
	"      and determine its size."
);

U_BOOT_CMD(ext4ls, 4, 1, do_ext4_ls,
	   "list files in a directory (default /)",
	   "<interface> <dev[:part]> [directory]\n"
	   "    - list files from 'dev' on 'interface' in a 'directory'");

U_BOOT_CMD(ext4load, 7, 0, do_ext4_load,
	   "load binary file from a Ext4 filesystem",
	   "<interface> [<dev[:part]> [addr [filename [bytes [pos]]]]]\n"
	   "    - load binary file 'filename' from 'dev' on 'interface'\n"
	   "      to address 'addr' from ext4 filesystem");
