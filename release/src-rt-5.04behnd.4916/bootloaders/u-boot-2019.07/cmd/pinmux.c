// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>

#define LIMIT_DEVNAME	30

static struct udevice *currdev;

static int do_dev(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *name;
	int ret;

	switch (argc) {
	case 2:
		name = argv[1];
		ret = uclass_get_device_by_name(UCLASS_PINCTRL, name, &currdev);
		if (ret) {
			printf("Can't get the pin-controller: %s!\n", name);
			return CMD_RET_FAILURE;
		}
	case 1:
		if (!currdev) {
			printf("Pin-controller device is not set!\n");
			return CMD_RET_USAGE;
		}

		printf("dev: %s\n", currdev->name);
	}

	return CMD_RET_SUCCESS;
}

static int show_pinmux(struct udevice *dev)
{
	char pin_name[PINNAME_SIZE];
	char pin_mux[PINMUX_SIZE];
	int pins_count;
	int i;
	int ret;

	pins_count = pinctrl_get_pins_count(dev);

	if (pins_count == -ENOSYS) {
		printf("Ops get_pins_count not supported\n");
		return pins_count;
	}

	for (i = 0; i < pins_count; i++) {
		ret = pinctrl_get_pin_name(dev, i, pin_name, PINNAME_SIZE);
		if (ret == -ENOSYS) {
			printf("Ops get_pin_name not supported\n");
			return ret;
		}

		ret = pinctrl_get_pin_muxing(dev, i, pin_mux, PINMUX_SIZE);
		if (ret) {
			printf("Ops get_pin_muxing error (%d)\n", ret);
			return ret;
		}

		printf("%-*s: %-*s\n", PINNAME_SIZE, pin_name,
		       PINMUX_SIZE, pin_mux);
	}

	return 0;
}

static int do_status(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int ret = CMD_RET_USAGE;

	if (currdev && (argc < 2 || strcmp(argv[1], "-a")))
		return show_pinmux(currdev);

	if (argc < 2 || strcmp(argv[1], "-a"))
		return ret;

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		/* insert a separator between each pin-controller display */
		printf("--------------------------\n");
		printf("%s:\n", dev->name);
		ret = show_pinmux(dev);
		if (ret < 0)
			printf("Can't display pin muxing for %s\n",
			       dev->name);
	}

	return ret;
}

static int do_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;

	printf("| %-*.*s| %-*.*s| %s\n",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Device",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Driver",
	       "Parent");

	uclass_foreach_dev_probe(UCLASS_PINCTRL, dev) {
		printf("| %-*.*s| %-*.*s| %s\n",
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->name,
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->driver->name,
		       dev->parent->name);
	}

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t pinmux_subcmd[] = {
	U_BOOT_CMD_MKENT(dev, 2, 1, do_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(status, 2, 1, do_status, "", ""),
};

static int do_pinmux(cmd_tbl_t *cmdtp, int flag, int argc,
		     char * const argv[])
{
	cmd_tbl_t *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], pinmux_subcmd, ARRAY_SIZE(pinmux_subcmd));
	if (!cmd || argc > cmd->maxargs)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(pinmux, CONFIG_SYS_MAXARGS, 1, do_pinmux,
	   "show pin-controller muxing",
	   "list                     - list UCLASS_PINCTRL devices\n"
	   "pinmux dev [pincontroller-name] - select pin-controller device\n"
	   "pinmux status [-a]              - print pin-controller muxing [for all]\n"
)
