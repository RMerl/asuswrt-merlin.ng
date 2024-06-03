// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 */


/*
 * UBIFS command support
 */

#undef DEBUG

#include <common.h>
#include <config.h>
#include <command.h>
#include <ubifs_uboot.h>

static int ubifs_initialized;
static int ubifs_mounted;

int cmd_ubifs_mount(char *vol_name)
{
	int ret;

	debug("Using volume %s\n", vol_name);

	if (ubifs_initialized == 0) {
		ubifs_init();
		ubifs_initialized = 1;
	}

	ret = uboot_ubifs_mount(vol_name);
	if (ret)
		return -1;

	ubifs_mounted = 1;

	return ret;
}
static int do_ubifs_mount(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	char *vol_name;

	if (argc != 2)
		return CMD_RET_USAGE;

	vol_name = argv[1];

	return cmd_ubifs_mount(vol_name);
}

int ubifs_is_mounted(void)
{
	return ubifs_mounted;
}

int cmd_ubifs_umount(void)
{
	if (ubifs_initialized == 0) {
		printf("No UBIFS volume mounted!\n");
		return -1;
	}

	uboot_ubifs_umount();
	ubifs_mounted = 0;
	ubifs_initialized = 0;

	return 0;
}

static int do_ubifs_umount(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	if (argc != 1)
		return CMD_RET_USAGE;

	return cmd_ubifs_umount();
}

static int do_ubifs_ls(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	char *filename = "/";
	int ret;

	if (!ubifs_mounted) {
		printf("UBIFS not mounted, use ubifsmount to mount volume first!\n");
		return -1;
	}

	if (argc == 2)
		filename = argv[1];
	debug("Using filename %s\n", filename);

	ret = ubifs_ls(filename);
	if (ret) {
		printf("** File not found %s **\n", filename);
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

static int do_ubifs_load(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	char *filename;
	char *endp;
	int ret;
	u32 addr;
	u32 size = 0;

	if (!ubifs_mounted) {
		printf("UBIFS not mounted, use ubifs mount to mount volume first!\n");
		return -1;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &endp, 16);
	if (endp == argv[1])
		return CMD_RET_USAGE;

	filename = argv[2];

	if (argc == 4) {
		size = simple_strtoul(argv[3], &endp, 16);
		if (endp == argv[3])
			return CMD_RET_USAGE;
	}
	debug("Loading file '%s' to address 0x%08x (size %d)\n", filename, addr, size);

	ret = ubifs_load(filename, addr, size);
	if (ret) {
		printf("** File not found %s **\n", filename);
		ret = CMD_RET_FAILURE;
	}

	return ret;
}

U_BOOT_CMD(
	ubifsmount, 2, 0, do_ubifs_mount,
	"mount UBIFS volume",
	"<volume-name>\n"
	"    - mount 'volume-name' volume"
);

U_BOOT_CMD(
	ubifsumount, 1, 0, do_ubifs_umount,
	"unmount UBIFS volume",
	"    - unmount current volume"
);

U_BOOT_CMD(
	ubifsls, 2, 0, do_ubifs_ls,
	"list files in a directory",
	"[directory]\n"
	"    - list files in a 'directory' (default '/')"
);

U_BOOT_CMD(
	ubifsload, 4, 0, do_ubifs_load,
	"load file from an UBIFS filesystem",
	"<addr> <filename> [bytes]\n"
	"    - load file 'filename' to address 'addr'"
);
