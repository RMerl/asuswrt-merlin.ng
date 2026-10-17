// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm-demo.h>
#include <mapmem.h>
#include <asm/io.h>

struct udevice *demo_dev;

static int do_demo_hello(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ch = 0;

	if (argc)
		ch = *argv[0];

	return demo_hello(demo_dev, ch);
}

static int do_demo_status(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	int status;
	int ret;

	ret = demo_status(demo_dev, &status);
	if (ret)
		return ret;

	printf("Status: %d\n", status);

	return 0;
}

static int do_demo_light(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int light;
	int ret;

	if (argc) {
		light = hextoul(argv[0], NULL);
		ret = demo_set_light(demo_dev, light);
	} else {
		ret = demo_get_light(demo_dev);
		if (ret >= 0) {
			printf("Light: %x\n", ret);
			ret = 0;
		}
	}

	return ret;
}

int do_demo_list(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	int i, ret, err = 0;

	puts("Demo uclass entries:\n");

	for (i = 0, ret = uclass_first_device_check(UCLASS_DEMO, &dev);
	     dev;
	     ret = uclass_next_device_check(&dev)) {
		printf("entry %d - instance %08x, ops %08x, plat %08x, status %i\n",
		       i++, (uint)map_to_sysmem(dev),
		       (uint)map_to_sysmem(dev->driver->ops),
		       (uint)map_to_sysmem(dev_get_plat(dev)),
		       ret);
		if (ret)
			err = ret;
	}

	return cmd_process_error(cmdtp, err);
}

static struct cmd_tbl demo_commands[] = {
	U_BOOT_CMD_MKENT(list, 0, 1, do_demo_list, "", ""),
	U_BOOT_CMD_MKENT(hello, 2, 1, do_demo_hello, "", ""),
	U_BOOT_CMD_MKENT(light, 2, 1, do_demo_light, "", ""),
	U_BOOT_CMD_MKENT(status, 1, 1, do_demo_status, "", ""),
};

static int do_demo(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct cmd_tbl *demo_cmd;
	int devnum = 0;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	demo_cmd = find_cmd_tbl(argv[1], demo_commands,
				ARRAY_SIZE(demo_commands));
	argc -= 2;
	argv += 2;

	if ((!demo_cmd || argc > demo_cmd->maxargs) ||
	    ((demo_cmd->name[0] != 'l') && (argc < 1)))
		return CMD_RET_USAGE;

	if (argc) {
		devnum = dectoul(argv[0], NULL);
		ret = uclass_get_device(UCLASS_DEMO, devnum, &demo_dev);
		if (ret)
			return cmd_process_error(cmdtp, ret);
		argc--;
		argv++;
	} else {
		demo_dev = NULL;
		if (demo_cmd->cmd != do_demo_list)
			return CMD_RET_USAGE;
	}

	ret = demo_cmd->cmd(demo_cmd, flag, argc, argv);

	return cmd_process_error(demo_cmd, ret);
}

U_BOOT_CMD(
	demo,   4,      1,      do_demo,
	"Driver model (dm) demo operations",
	"list                     List available demo devices\n"
	"demo hello <num> [<char>]     Say hello\n"
	"demo light [<num>]            Set or get the lights\n"
	"demo status <num>             Get demo device status\n"
);
