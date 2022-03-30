// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
 */

/*
 * CBFS commands
 */
#include <common.h>
#include <command.h>
#include <cbfs.h>

static int do_cbfs_init(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	uintptr_t end_of_rom = 0xffffffff;
	char *ep;

	if (argc > 2) {
		printf("usage: cbfsls [end of rom]>\n");
		return 0;
	}
	if (argc == 2) {
		end_of_rom = simple_strtoul(argv[1], &ep, 16);
		if (*ep) {
			puts("\n** Invalid end of ROM **\n");
			return 1;
		}
	}
	file_cbfs_init(end_of_rom);
	if (file_cbfs_result != CBFS_SUCCESS) {
		printf("%s.\n", file_cbfs_error());
		return 1;
	}
	return 0;
}

U_BOOT_CMD(
	cbfsinit,	2,	0,	do_cbfs_init,
	"initialize the cbfs driver",
	"[end of rom]\n"
	"    - Initialize the cbfs driver. The optional 'end of rom'\n"
	"      parameter specifies where the end of the ROM is that the\n"
	"      CBFS is in. It defaults to 0xFFFFFFFF\n"
);

static int do_cbfs_fsload(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	const struct cbfs_cachenode *file;
	unsigned long offset;
	unsigned long count;
	long size;

	if (argc < 3) {
		printf("usage: cbfsload <addr> <filename> [bytes]\n");
		return 1;
	}

	/* parse offset and count */
	offset = simple_strtoul(argv[1], NULL, 16);
	if (argc == 4)
		count = simple_strtoul(argv[3], NULL, 16);
	else
		count = 0;

	file = file_cbfs_find(argv[2]);
	if (!file) {
		if (file_cbfs_result == CBFS_FILE_NOT_FOUND)
			printf("%s: %s\n", file_cbfs_error(), argv[2]);
		else
			printf("%s.\n", file_cbfs_error());
		return 1;
	}

	printf("reading %s\n", file_cbfs_name(file));

	size = file_cbfs_read(file, (void *)offset, count);

	printf("\n%ld bytes read\n", size);

	env_set_hex("filesize", size);

	return 0;
}

U_BOOT_CMD(
	cbfsload,	4,	0,	do_cbfs_fsload,
	"load binary file from a cbfs filesystem",
	"<addr> <filename> [bytes]\n"
	"    - load binary file 'filename' from the cbfs to address 'addr'\n"
);

static int do_cbfs_ls(cmd_tbl_t *cmdtp, int flag, int argc,
		      char *const argv[])
{
	const struct cbfs_cachenode *file = file_cbfs_get_first();
	int files = 0;

	if (!file) {
		printf("%s.\n", file_cbfs_error());
		return 1;
	}

	printf("     size              type  name\n");
	printf("------------------------------------------\n");
	while (file) {
		int type = file_cbfs_type(file);
		char *type_name = NULL;
		const char *filename = file_cbfs_name(file);

		printf(" %8d", file_cbfs_size(file));

		switch (type) {
		case CBFS_TYPE_BOOTBLOCK:
			type_name = "bootblock";
			break;
		case CBFS_TYPE_CBFSHEADER:
			type_name = "cbfs header";
			break;
		case CBFS_TYPE_STAGE:
			type_name = "stage";
			break;
		case CBFS_TYPE_PAYLOAD:
			type_name = "payload";
			break;
		case CBFS_TYPE_FIT:
			type_name = "fit";
			break;
		case CBFS_TYPE_OPTIONROM:
			type_name = "option rom";
			break;
		case CBFS_TYPE_BOOTSPLASH:
			type_name = "boot splash";
			break;
		case CBFS_TYPE_RAW:
			type_name = "raw";
			break;
		case CBFS_TYPE_VSA:
			type_name = "vsa";
			break;
		case CBFS_TYPE_MBI:
			type_name = "mbi";
			break;
		case CBFS_TYPE_MICROCODE:
			type_name = "microcode";
			break;
		case CBFS_TYPE_FSP:
			type_name = "fsp";
			break;
		case CBFS_TYPE_MRC:
			type_name = "mrc";
			break;
		case CBFS_TYPE_MMA:
			type_name = "mma";
			break;
		case CBFS_TYPE_EFI:
			type_name = "efi";
			break;
		case CBFS_TYPE_STRUCT:
			type_name = "struct";
			break;
		case CBFS_TYPE_CMOS_DEFAULT:
			type_name = "cmos default";
			break;
		case CBFS_TYPE_SPD:
			type_name = "spd";
			break;
		case CBFS_TYPE_MRC_CACHE:
			type_name = "mrc cache";
			break;
		case CBFS_TYPE_CMOS_LAYOUT:
			type_name = "cmos layout";
			break;
		case -1:
		case 0:
			type_name = "null";
			break;
		}
		if (type_name)
			printf("  %16s", type_name);
		else
			printf("  %16d", type);

		if (filename[0])
			printf("  %s\n", filename);
		else
			printf("  %s\n", "(empty)");
		file_cbfs_get_next(&file);
		files++;
	}

	printf("\n%d file(s)\n\n", files);
	return 0;
}

U_BOOT_CMD(
	cbfsls,	1,	1,	do_cbfs_ls,
	"list files",
	"    - list the files in the cbfs\n"
);

static int do_cbfs_fsinfo(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	const struct cbfs_header *header = file_cbfs_get_header();

	if (!header) {
		printf("%s.\n", file_cbfs_error());
		return 1;
	}

	printf("\n");
	printf("CBFS version: %#x\n", header->version);
	printf("ROM size: %#x\n", header->rom_size);
	printf("Boot block size: %#x\n", header->boot_block_size);
	printf("CBFS size: %#x\n",
		header->rom_size - header->boot_block_size - header->offset);
	printf("Alignment: %d\n", header->align);
	printf("Offset: %#x\n", header->offset);
	printf("\n");

	return 0;
}

U_BOOT_CMD(
	cbfsinfo,	1,	1,	do_cbfs_fsinfo,
	"print information about filesystem",
	"    - print information about the cbfs filesystem\n"
);
