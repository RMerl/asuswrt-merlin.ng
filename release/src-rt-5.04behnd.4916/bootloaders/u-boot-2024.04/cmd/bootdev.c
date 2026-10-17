// SPDX-License-Identifier: GPL-2.0+
/*
 * 'bootdev' command
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <bootdev.h>
#include <bootflow.h>
#include <bootstd.h>
#include <command.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

static int bootdev_check_state(struct bootstd_priv **stdp)
{
	struct bootstd_priv *std;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return ret;
	if (!std->cur_bootdev) {
		printf("Please use 'bootdev select' first\n");
		return -ENOENT;
	}
	*stdp = std;

	return 0;
}

static int do_bootdev_list(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	bool probe;

	probe = argc >= 2 && !strcmp(argv[1], "-p");
	bootdev_list(probe);

	return 0;
}

static int do_bootdev_select(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	struct bootstd_priv *std;
	struct udevice *dev;
	int ret;

	ret = bootstd_get_priv(&std);
	if (ret)
		return CMD_RET_FAILURE;
	if (argc < 2) {
		std->cur_bootdev = NULL;
		return 0;
	}
	if (bootdev_find_by_any(argv[1], &dev, NULL))
		return CMD_RET_FAILURE;

	std->cur_bootdev = dev;

	return 0;
}

static int do_bootdev_info(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct bootstd_priv *priv;
	struct bootflow *bflow;
	int ret, i, num_valid;
	struct udevice *dev;
	bool probe;

	probe = argc >= 2 && !strcmp(argv[1], "-p");

	ret = bootdev_check_state(&priv);
	if (ret)
		return CMD_RET_FAILURE;

	dev = priv->cur_bootdev;

	/* Count the number of bootflows, including how many are valid*/
	num_valid = 0;
	for (ret = bootdev_first_bootflow(dev, &bflow), i = 0;
	     !ret;
	     ret = bootdev_next_bootflow(&bflow), i++)
		num_valid += bflow->state == BOOTFLOWST_READY;

	/*
	 * Prove the device, if requested, otherwise assume that there is no
	 * error
	 */
	ret = 0;
	if (probe)
		ret = device_probe(dev);

	printf("Name:      %s\n", dev->name);
	printf("Sequence:  %d\n", dev_seq(dev));
	printf("Status:    %s\n", ret ? simple_itoa(-ret) : device_active(dev) ?
		"Probed" : "OK");
	printf("Uclass:    %s\n", dev_get_uclass_name(dev_get_parent(dev)));
	printf("Bootflows: %d (%d valid)\n", i, num_valid);

	return 0;
}

static int do_bootdev_hunt(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct bootstd_priv *priv;
	const char *spec = NULL;
	bool list = false;
	int ret = 0;

	if (argc >= 2) {
		if (!strcmp(argv[1], "-l"))
			list = true;
		else
			spec = argv[1];
	}

	ret = bootstd_get_priv(&priv);
	if (ret)
		return ret;
	if (list) {
		bootdev_list_hunters(priv);
	} else {
		ret = bootdev_hunt(spec, true);
		if (ret) {
			printf("Failed (err=%dE)\n", ret);

			return CMD_RET_FAILURE;
		}
	}

	return 0;
}

U_BOOT_LONGHELP(bootdev,
	"list [-p]         - list all available bootdevs (-p to probe)\n"
	"bootdev hunt [-l|<spec>]  - use hunt drivers to find bootdevs\n"
	"bootdev select <bd>       - select a bootdev by name | label | seq\n"
	"bootdev info [-p]         - show information about a bootdev (-p to probe)");

U_BOOT_CMD_WITH_SUBCMDS(bootdev, "Boot devices", bootdev_help_text,
	U_BOOT_SUBCMD_MKENT(list, 2, 1, do_bootdev_list),
	U_BOOT_SUBCMD_MKENT(hunt, 2, 1, do_bootdev_hunt),
	U_BOOT_SUBCMD_MKENT(select, 2, 1, do_bootdev_select),
	U_BOOT_SUBCMD_MKENT(info, 2, 1, do_bootdev_info));
