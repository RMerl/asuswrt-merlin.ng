// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <command.h>
#include <env.h>
#include <fs.h>
#include <pxe_utils.h>

/**
 * struct sysboot_info - useful information for sysboot helpers
 *
 * @fstype: Filesystem type (FS_TYPE_...)
 * @ifname: Interface name (e.g. "ide", "scsi")
 * @dev_part_str is in the format:
 *	<dev>.<hw_part>:<part> where <dev> is the device number,
 *	<hw_part> is the optional hardware partition number and
 *	<part> is the partition number
 */
struct sysboot_info {
	int fstype;
	const char *ifname;
	const char *dev_part_str;
};

static int sysboot_read_file(struct pxe_context *ctx, const char *file_path,
			     char *file_addr, ulong *sizep)
{
	struct sysboot_info *info = ctx->userdata;
	loff_t len_read;
	ulong addr;
	int ret;

	addr = simple_strtoul(file_addr, NULL, 16);
	ret = fs_set_blk_dev(info->ifname, info->dev_part_str, info->fstype);
	if (ret)
		return ret;
	ret = fs_read(file_path, addr, 0, 0, &len_read);
	if (ret)
		return ret;
	*sizep = len_read;

	return 0;
}

/*
 * Boots a system using a local disk syslinux/extlinux file
 *
 * Returns 0 on success, 1 on error.
 */
static int do_sysboot(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	unsigned long pxefile_addr_r;
	struct pxe_context ctx;
	char *pxefile_addr_str;
	struct sysboot_info info;
	char *filename;
	int prompt = 0;
	int ret;

	if (argc > 1 && strstr(argv[1], "-p")) {
		prompt = 1;
		argc--;
		argv++;
	}

	if (argc < 4)
		return cmd_usage(cmdtp);

	if (argc < 5) {
		pxefile_addr_str = from_env("pxefile_addr_r");
		if (!pxefile_addr_str)
			return 1;
	} else {
		pxefile_addr_str = argv[4];
	}

	if (argc < 6) {
		filename = env_get("bootfile");
	} else {
		filename = argv[5];
		env_set("bootfile", filename);
	}

	if (strstr(argv[3], "ext2")) {
		info.fstype = FS_TYPE_EXT;
	} else if (strstr(argv[3], "fat")) {
		info.fstype = FS_TYPE_FAT;
	} else if (strstr(argv[3], "any")) {
		info.fstype = FS_TYPE_ANY;
	} else {
		printf("Invalid filesystem: %s\n", argv[3]);
		return 1;
	}
	info.ifname = argv[1];
	info.dev_part_str = argv[2];

	if (strict_strtoul(pxefile_addr_str, 16, &pxefile_addr_r) < 0) {
		printf("Invalid pxefile address: %s\n", pxefile_addr_str);
		return 1;
	}

	if (pxe_setup_ctx(&ctx, cmdtp, sysboot_read_file, &info, true,
			  filename, false)) {
		printf("Out of memory\n");
		return CMD_RET_FAILURE;
	}

	if (get_pxe_file(&ctx, filename, pxefile_addr_r) < 0) {
		printf("Error reading config file\n");
		pxe_destroy_ctx(&ctx);
		return 1;
	}

	ret = pxe_process(&ctx, pxefile_addr_r, prompt);
	pxe_destroy_ctx(&ctx);
	if (ret)
		return CMD_RET_FAILURE;

	return 0;
}

U_BOOT_CMD(sysboot, 7, 1, do_sysboot,
	   "command to get and boot from syslinux files",
	   "[-p] <interface> <dev[:part]> <ext2|fat|any> [addr] [filename]\n"
	   "    - load and parse syslinux menu file 'filename' from ext2, fat\n"
	   "      or any filesystem on 'dev' on 'interface' to address 'addr'"
);
