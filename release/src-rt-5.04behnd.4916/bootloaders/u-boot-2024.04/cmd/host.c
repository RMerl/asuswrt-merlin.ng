// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, Google Inc.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <fs.h>
#include <part.h>
#include <sandbox_host.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <linux/errno.h>
#include <linux/log2.h>

static int do_host_load(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return do_load(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_ls(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	return do_ls(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_size(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return do_size(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_save(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return do_save(cmdtp, flag, argc, argv, FS_TYPE_SANDBOX);
}

static int do_host_bind(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	bool removable = false;
	struct udevice *dev;
	const char *label;
	char *file;
	unsigned long blksz = DEFAULT_BLKSZ;
	int ret;

	/* Skip 'bind' */
	argc--;
	argv++;
	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[0], "-r")) {
		removable = true;
		argc--;
		argv++;
	}

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;
	label = argv[0];
	file = argv[1];
	if (argc > 2) {
		blksz = dectoul(argv[2], NULL);
		if (blksz < DEFAULT_BLKSZ || !is_power_of_2(blksz)) {
			printf("blksz must be >= 512 and power of 2\n");
			return CMD_RET_FAILURE;
		}
	}

	ret = host_create_attach_file(label, file, removable, blksz, &dev);
	if (ret) {
		printf("Cannot create device / bind file\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

/**
 * parse_host_label() - Parse a device label or sequence number
 *
 * This shows an error if it returns NULL
 *
 * @label: String containing the label or sequence number
 * Returns: Associated device, or NULL if not found
 */
static struct udevice *parse_host_label(const char *label)
{
	struct udevice *dev;

	dev = host_find_by_label(label);
	if (!dev) {
		int devnum;
		char *ep;

		devnum = hextoul(label, &ep);
		if (*ep ||
		    uclass_find_device_by_seq(UCLASS_HOST, devnum, &dev)) {
			printf("No such device '%s'\n", label);
			return NULL;
		}
	}

	return dev;
}

static int do_host_unbind(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct udevice *dev;
	const char *label;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	label = argv[1];
	dev = parse_host_label(label);
	if (!dev)
		return CMD_RET_FAILURE;

	ret = host_detach_file(dev);
	if (ret) {
		printf("Cannot detach file (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = device_unbind(dev);
	if (ret) {
		printf("Cannot attach file\n");
		ret = device_unbind(dev);
		if (ret)
			printf("Cannot unbind device '%s'\n", dev->name);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static void show_host_dev(struct udevice *dev)
{
	struct host_sb_plat *plat = dev_get_plat(dev);
	struct blk_desc *desc;
	struct udevice *blk;
	int ret;

	printf("%3d ", dev_seq(dev));
	if (!plat->fd) {
		printf("Not bound to a backing file\n");
		return;
	}
	ret = blk_get_from_parent(dev, &blk);
	if (ret)  /* cannot happen */
		return;

	desc = dev_get_uclass_plat(blk);
	printf("%12lu %6lu %-15s %s\n", (unsigned long)desc->lba, desc->blksz,
	       plat->label, plat->filename);
}

static int do_host_info(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct udevice *dev;

	if (argc < 1)
		return CMD_RET_USAGE;

	dev = NULL;
	if (argc >= 2) {
		dev = parse_host_label(argv[1]);
		if (!dev)
			return CMD_RET_FAILURE;
	}

	printf("%3s %12s %6s %-15s %s\n",
	       "dev", "blocks", "blksz", "label", "path");
	if (dev) {
		show_host_dev(dev);
	} else {
		struct uclass *uc;

		uclass_id_foreach_dev(UCLASS_HOST, dev, uc)
			show_host_dev(dev);
	}

	return 0;
}

static int do_host_dev(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	const char *label;

	if (argc < 1 || argc > 3)
		return CMD_RET_USAGE;

	if (argc == 1) {
		struct host_sb_plat *plat;

		dev = host_get_cur_dev();
		if (!dev) {
			printf("No current host device\n");
			return CMD_RET_FAILURE;
		}
		plat = dev_get_plat(dev);
		printf("Current host device: %d: %s\n", dev_seq(dev),
		       plat->label);
		return 0;
	}

	label = argv[1];
	dev = parse_host_label(argv[1]);
	if (!dev)
		return CMD_RET_FAILURE;

	host_set_cur_dev(dev);

	return 0;
}

static struct cmd_tbl cmd_host_sub[] = {
	U_BOOT_CMD_MKENT(load, 7, 0, do_host_load, "", ""),
	U_BOOT_CMD_MKENT(ls, 3, 0, do_host_ls, "", ""),
	U_BOOT_CMD_MKENT(save, 6, 0, do_host_save, "", ""),
	U_BOOT_CMD_MKENT(size, 3, 0, do_host_size, "", ""),
	U_BOOT_CMD_MKENT(bind, 4, 0, do_host_bind, "", ""),
	U_BOOT_CMD_MKENT(unbind, 4, 0, do_host_unbind, "", ""),
	U_BOOT_CMD_MKENT(info, 3, 0, do_host_info, "", ""),
	U_BOOT_CMD_MKENT(dev, 0, 1, do_host_dev, "", ""),
};

static int do_host(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct cmd_tbl *c;

	/* Skip past 'host' */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_host_sub, ARRAY_SIZE(cmd_host_sub));

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
	"host bind [-r] <label> <filename> [<blksz>] - bind \"host\" device to file,\n"
	"     and optionally set the device's logical block size\n"
	"     -r = mark as removable\n"
	"host unbind <label>     - unbind file from \"host\" device\n"
	"host info [<label>]     - show device binding & info\n"
	"host dev [<label>]      - set or retrieve the current host device\n"
	"host commands use the \"hostfs\" device. The \"host\" device is used\n"
	"with standard IO commands such as fatls or ext2load"
);
