// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) 2022 Sartura Ltd.
 * Written by Robert Marko <robert.marko@sartura.hr>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <thermal.h>

#define LIMIT_DEVNAME	30

static int do_get(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	struct udevice *dev;
	int ret, temp;

	if (argc < 2) {
		printf("thermal device not selected\n");
		return CMD_RET_FAILURE;
	}

	ret = uclass_get_device_by_name(UCLASS_THERMAL, argv[1], &dev);
	if (ret) {
		printf("thermal device not found\n");
		return CMD_RET_FAILURE;
	}

	ret = thermal_get_temp(dev, &temp);
	if (ret)
		return CMD_RET_FAILURE;

	printf("%s: %d C\n", dev->name, temp);

	return CMD_RET_SUCCESS;
}

static int do_list(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct udevice *dev;

	printf("| %-*.*s| %-*.*s| %s\n",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Device",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Driver",
	       "Parent");

	uclass_foreach_dev_probe(UCLASS_THERMAL, dev) {
		printf("| %-*.*s| %-*.*s| %s\n",
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->name,
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->driver->name,
		       dev->parent->name);
	}

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl temperature_subcmd[] = {
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(get, 2, 1, do_get, "", ""),
};

static int do_temperature(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	struct cmd_tbl *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], temperature_subcmd, ARRAY_SIZE(temperature_subcmd));
	if (!cmd || argc > cmd->maxargs)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(temperature, CONFIG_SYS_MAXARGS, 1, do_temperature,
	   "thermal sensor temperature",
	   "list\t\tshow list of temperature sensors\n"
	   "get [thermal device name]\tprint temperature in degrees C"
);
