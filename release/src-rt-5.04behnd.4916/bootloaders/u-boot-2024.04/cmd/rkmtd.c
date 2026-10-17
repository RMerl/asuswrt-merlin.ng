// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Driver interface derived from:
 * /cmd/host.c
 * Copyright (c) 2012, Google Inc.
 *
 * Copyright (C) 2023 Johan Jonker <jbx6244@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <dm.h>
#include <rkmtd.h>
#include <stdio.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

static int do_rkmtd_bind(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct udevice *dev;
	const char *label;
	int ret;

	argc--;
	argv++;

	if (argc < 1)
		return CMD_RET_USAGE;

	if (argc > 1)
		return CMD_RET_USAGE;

	label = argv[0];
	ret = rkmtd_create_attach_mtd(label, &dev);
	if (ret) {
		printf("Cannot create device / bind mtd\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

static struct udevice *parse_rkmtd_label(const char *label)
{
	struct udevice *dev;

	dev = rkmtd_find_by_label(label);
	if (!dev) {
		int devnum;
		char *ep;

		devnum = hextoul(label, &ep);
		if (*ep ||
		    uclass_find_device_by_seq(UCLASS_RKMTD, devnum, &dev)) {
			printf("No such device '%s'\n", label);
			return NULL;
		}
	}

	return dev;
}

static int do_rkmtd_unbind(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct udevice *dev;
	const char *label;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	label = argv[1];
	dev = parse_rkmtd_label(label);
	if (!dev)
		return CMD_RET_FAILURE;

	ret = rkmtd_detach(dev);
	if (ret) {
		printf("Cannot detach mtd\n");
		return CMD_RET_FAILURE;
	}

	ret = device_unbind(dev);
	if (ret) {
		printf("Cannot unbind device '%s'\n", dev->name);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static void show_rkmtd_dev(struct udevice *dev)
{
	struct rkmtd_dev *plat = dev_get_plat(dev);
	struct blk_desc *desc;
	struct udevice *blk;
	int ret;

	printf("%3d ", dev_seq(dev));

	ret = blk_get_from_parent(dev, &blk);
	if (ret)
		return;

	desc = dev_get_uclass_plat(blk);
	printf("%12lu %-15s\n", (unsigned long)desc->lba, plat->label);
}

static int do_rkmtd_info(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct udevice *dev;

	if (argc < 1)
		return CMD_RET_USAGE;

	dev = NULL;
	if (argc >= 2) {
		dev = parse_rkmtd_label(argv[1]);
		if (!dev)
			return CMD_RET_FAILURE;
	}

	printf("%3s %12s %-15s\n", "dev", "blocks", "label");
	if (dev) {
		show_rkmtd_dev(dev);
	} else {
		struct uclass *uc;

		uclass_id_foreach_dev(UCLASS_RKMTD, dev, uc)
			show_rkmtd_dev(dev);
	}

	return 0;
}

static int do_rkmtd_dev(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct udevice *dev;
	const char *label;

	if (argc < 1 || argc > 3)
		return CMD_RET_USAGE;

	if (argc == 1) {
		struct rkmtd_dev *plat;

		dev = rkmtd_get_cur_dev();
		if (!dev) {
			printf("No current rkmtd device\n");
			return CMD_RET_FAILURE;
		}
		plat = dev_get_plat(dev);
		printf("Current rkmtd device: %d: %s\n", dev_seq(dev),
		       plat->label);
		return 0;
	}

	label = argv[1];
	dev = parse_rkmtd_label(argv[1]);
	if (!dev)
		return CMD_RET_FAILURE;

	rkmtd_set_cur_dev(dev);

	return 0;
}

static struct cmd_tbl cmd_rkmtd_sub[] = {
	U_BOOT_CMD_MKENT(bind, 4, 0, do_rkmtd_bind, "", ""),
	U_BOOT_CMD_MKENT(unbind, 4, 0, do_rkmtd_unbind, "", ""),
	U_BOOT_CMD_MKENT(info, 3, 0, do_rkmtd_info, "", ""),
	U_BOOT_CMD_MKENT(dev, 0, 1, do_rkmtd_dev, "", ""),
};

static int do_rkmtd(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct cmd_tbl *c;

	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_rkmtd_sub, ARRAY_SIZE(cmd_rkmtd_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	rkmtd, 8, 1, do_rkmtd,
	"Rockchip MTD sub-system",
	"bind <label>      - bind RKMTD device\n"
	"rkmtd unbind <label>    - unbind RKMTD device\n"
	"rkmtd info [<label>]    - show all available RKMTD devices\n"
	"rkmtd dev [<label>]     - show or set current RKMTD device\n"
);
