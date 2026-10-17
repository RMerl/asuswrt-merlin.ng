// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022
 * Roger Knecht <rknecht@pm.de>
 */

#include <common.h>
#include <command.h>
#include <display_options.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>

static int do_xxd(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	char *ifname;
	char *dev;
	char *file;
	char *buffer;
	phys_addr_t addr;
	loff_t file_size;

	if (argc < 4)
		return CMD_RET_USAGE;

	ifname = argv[1];
	dev = argv[2];
	file = argv[3];

	// check file exists
	if (fs_set_blk_dev(ifname, dev, FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	if (!fs_exists(file)) {
		log_err("File does not exist: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// get file size
	if (fs_set_blk_dev(ifname, dev, FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	if (fs_size(file, &file_size)) {
		log_err("Cannot read file size: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// allocate memory for file content
	buffer = calloc(sizeof(char), file_size);
	if (!buffer) {
		log_err("Out of memory\n");
		return CMD_RET_FAILURE;
	}

	// map pointer to system memory
	addr = map_to_sysmem(buffer);

	// read file to memory
	if (fs_set_blk_dev(ifname, dev, FS_TYPE_ANY))
		return CMD_RET_FAILURE;

	if (fs_read(file, addr, 0, 0, &file_size)) {
		log_err("Cannot read file: ifname=%s dev=%s file=%s\n", ifname, dev, file);
		return CMD_RET_FAILURE;
	}

	// print file content
	print_buffer(0, buffer, sizeof(char), file_size, 0);

	free(buffer);

	return 0;
}

U_BOOT_LONGHELP(xxd,
	"<interface> <dev[:part]> <file>\n"
	"  - Print file from 'dev' on 'interface' as hexdump to standard output\n");

U_BOOT_CMD(xxd, 4, 1, do_xxd,
	   "Print file as hexdump to standard output",
	   xxd_help_text
);
