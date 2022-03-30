// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <dm-demo.h>
#include <mapmem.h>
#include <asm/io.h>

struct udevice *demo_dev;

static int do_demo_hello(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	int ch = 0;

	if (argc)
		ch = *argv[0];

	return demo_hello(demo_dev, ch);
}

static int do_demo_status(cmd_tbl_t *cmdtp, int flag, int argc,
			  char * const argv[])
{
	int status;
	int ret;

	ret = demo_status(demo_dev, &status);
	if (ret)
		return ret;

	printf("Status: %d\n", status);

	return 0;
}

static int do_demo_light(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	int light;
	int ret;

	if (argc) {
		light = simple_strtoul(argv[0], NULL, 16);
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

int do_demo_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int i, ret;

	puts("Demo uclass entries:\n");

	for (i = 0, ret = uclass_first_device(UCLASS_DEMO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		printf("entry %d - instance %08x, ops %08x, platdata %08x\n",
		       i++, (uint)map_to_sysmem(dev),
		       (uint)map_to_sysmem(dev->driver->ops),
		       (uint)map_to_sysmem(dev_get_platdata(dev)));
	}

	return cmd_process_error(cmdtp, ret);
}

static cmd_tbl_t demo_commands[] = {
	U_BOOT_CMD_MKENT(list, 0, 1, do_demo_list, "", ""),
	U_BOOT_CMD_MKENT(hello, 2, 1, do_demo_hello, "", ""),
	U_BOOT_CMD_MKENT(light, 2, 1, do_demo_light, "", ""),
	U_BOOT_CMD_MKENT(status, 1, 1, do_demo_status, "", ""),
};

static int do_demo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *demo_cmd;
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
		devnum = simple_strtoul(argv[0], NULL, 10);
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
	"demo list                     List available demo devices"
);
