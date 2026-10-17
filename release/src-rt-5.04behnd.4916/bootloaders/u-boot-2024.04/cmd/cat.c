// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022
 * Roger Knecht <rknecht@pm.de>
 */

#include <common.h>
#include <command.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>

static int do_cat(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	char *ifname;
	char *dev;
	char *file;
	char *buffer;
	ulong file_size;
	int ret;

	if (argc < 4)
		return CMD_RET_USAGE;

	ifname = argv[1];
	dev = argv[2];
	file = argv[3];

	ret = fs_load_alloc(ifname, dev, file, 0, 0, (void **)&buffer,
			    &file_size);

	// check file exists
	switch (ret) {
	case 0:
		break;
	case -ENOMEDIUM:
		return CMD_RET_FAILURE;
	case -ENOENT:
		log_err("File does not exist: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	case -E2BIG:
		log_err("File is too large: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	case -ENOMEM:
		log_err("Not enough memory: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	default:
	case -EIO:
		log_err("File-read failed: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// print file content
	buffer[file_size] = '\0';
	puts(buffer);

	free(buffer);

	return 0;
}

U_BOOT_LONGHELP(cat,
	"<interface> <dev[:part]> <file>\n"
	"  - Print file from 'dev' on 'interface' to standard output\n");

U_BOOT_CMD(cat, 4, 1, do_cat,
	   "Print file to standard output",
	   cat_help_text
);
