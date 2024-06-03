// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, Google Inc.
 */

#include <common.h>
#include <dm.h>
#include <fs.h>
#include <part.h>
#include <sandboxblockdev.h>
#include <linux/errno.h>

static int host_curr_device = -1;

static int do_host_load(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_ls(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_size(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_size(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_save(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	return do_save(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_bind(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;
	char *ep;
	char *dev_str = argv[1];
	char *file = argc >= 3 ? argv[2] : NULL;
	int dev = simple_strtoul(dev_str, &ep, 16);
	if (*ep) {
		printf("** Bad device specification %s **\n", dev_str);
		return CMD_RET_USAGE;
	}
	return host_dev_bind(dev, file);
}

static int do_host_info(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	if (argc < 1 || argc > 2)
		return CMD_RET_USAGE;
	int min_dev = 0;
	int max_dev = CONFIG_HOST_MAX_DEVICES - 1;
	if (argc >= 2) {
		char *ep;
		char *dev_str = argv[1];
		int dev = simple_strtoul(dev_str, &ep, 16);
		if (*ep) {
			printf("** Bad device specification %s **\n", dev_str);
			return CMD_RET_USAGE;
		}
		min_dev = dev;
		max_dev = dev;
	}
	int dev;
	printf("%3s %12s %s\n", "dev", "blocks", "path");
	for (dev = min_dev; dev <= max_dev; dev++) {
		struct blk_desc *blk_dev;
		int ret;

		printf("%3d ", dev);
		ret = host_get_dev_err(dev, &blk_dev);
		if (ret) {
			if (ret == -ENOENT)
				puts("Not bound to a backing file\n");
			else if (ret == -ENODEV)
				puts("Invalid host device number\n");

			continue;
		}
		struct host_block_dev *host_dev;

#ifdef CONFIG_BLK
		host_dev = dev_get_priv(blk_dev->bdev);
#else
		host_dev = blk_dev->priv;
#endif
		printf("%12lu %s\n", (unsigned long)blk_dev->lba,
		       host_dev->filename);
	}
	return 0;
}

static int do_host_dev(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	int dev;
	char *ep;
	struct blk_desc *blk_dev;
	int ret;

	if (argc < 1 || argc > 3)
		return CMD_RET_USAGE;

	if (argc == 1) {
		if (host_curr_device < 0) {
			printf("No current host device\n");
			return 1;
		}
		printf("Current host device %d\n", host_curr_device);
		return 0;
	}

	dev = simple_strtoul(argv[1], &ep, 16);
	if (*ep) {
		printf("** Bad device specification %s **\n", argv[2]);
		return CMD_RET_USAGE;
	}

	ret = host_get_dev_err(dev, &blk_dev);
	if (ret) {
		if (ret == -ENOENT)
			puts("Not bound to a backing file\n");
		else if (ret == -ENODEV)
			puts("Invalid host device number\n");

		return 1;
	}

	host_curr_device = dev;
	return 0;
}

static cmd_tbl_t cmd_host_sub[] = {
	U_BOOT_CMD_MKENT(load, 7, 0, do_host_load, "", ""),
	U_BOOT_CMD_MKENT(ls, 3, 0, do_host_ls, "", ""),
	U_BOOT_CMD_MKENT(save, 6, 0, do_host_save, "", ""),
	U_BOOT_CMD_MKENT(size, 3, 0, do_host_size, "", ""),
	U_BOOT_CMD_MKENT(bind, 3, 0, do_host_bind, "", ""),
	U_BOOT_CMD_MKENT(info, 3, 0, do_host_info, "", ""),
	U_BOOT_CMD_MKENT(dev, 0, 1, do_host_dev, "", ""),
};

static int do_host(cmd_tbl_t *cmdtp, int flag, int argc,
		      char * const argv[])
{
	cmd_tbl_t *c;

	/* Skip past 'host' */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_host_sub,
			 ARRAY_SIZE(cmd_host_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	host, 8, 1, do_host,
	"Miscellaneous host commands",
	"load hostfs - <addr> <filename> [<bytes> <offset>]  - "
		"load a file from host\n"
	"host ls hostfs - <filename>                    - list files on host\n"
	"host save hostfs - <addr> <filename> <bytes> [<offset>] - "
		"save a file to host\n"
	"host size hostfs - <filename> - determine size of file on host\n"
	"host bind <dev> [<filename>] - bind \"host\" device to file\n"
	"host info [<dev>]            - show device binding & info\n"
	"host dev [<dev>] - Set or retrieve the current host device\n"
	"host commands use the \"hostfs\" device. The \"host\" device is used\n"
	"with standard IO commands such as fatls or ext2load"
);
