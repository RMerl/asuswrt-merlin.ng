// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Wind River Systems, Inc.
 *
 * Author:
 *   Bin Meng <bin.meng@windriver.com>
 *
 * A command interface to access misc devices with MISC uclass driver APIs.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>

enum misc_op {
	MISC_OP_READ,
	MISC_OP_WRITE
};

static char *misc_op_str[] = {
	"read",
	"write"
};

static int do_misc_list(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	struct udevice *dev;

	printf("Device               Index     Driver\n");
	printf("-------------------------------------\n");
	for (uclass_first_device(UCLASS_MISC, &dev);
	     dev;
	     uclass_next_device(&dev)) {
		printf("%-20s %5d %10s\n", dev->name, dev_seq(dev),
		       dev->driver->name);
	}

	return 0;
}

static int do_misc_op(struct cmd_tbl *cmdtp, int flag,
		      int argc, char *const argv[], enum misc_op op)
{
	struct udevice *dev;
	int offset;
	void *buf;
	int size;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_MISC, argv[0], &dev);
	if (ret) {
		printf("Unable to find device %s\n", argv[0]);
		return ret;
	}

	offset = hextoul(argv[1], NULL);
	buf = (void *)hextoul(argv[2], NULL);
	size = hextoul(argv[3], NULL);

	if (op == MISC_OP_READ)
		ret = misc_read(dev, offset, buf, size);
	else
		ret = misc_write(dev, offset, buf, size);

	if (ret < 0) {
		if (ret == -ENOSYS) {
			printf("The device does not support %s\n",
			       misc_op_str[op]);
			ret = 0;
		}
	} else {
		if (ret == size)
			ret = 0;
		else
			printf("Partially %s %d bytes\n", misc_op_str[op], ret);
	}

	return ret;
}

static int do_misc_read(struct cmd_tbl *cmdtp, int flag,
			int argc, char *const argv[])
{
	return do_misc_op(cmdtp, flag, argc, argv, MISC_OP_READ);
}

static int do_misc_write(struct cmd_tbl *cmdtp, int flag,
			 int argc, char *const argv[])
{
	return do_misc_op(cmdtp, flag, argc, argv, MISC_OP_WRITE);
}

static struct cmd_tbl misc_commands[] = {
	U_BOOT_CMD_MKENT(list, 0, 1, do_misc_list, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 1, do_misc_read, "", ""),
	U_BOOT_CMD_MKENT(write, 4, 1, do_misc_write, "", ""),
};

static int do_misc(struct cmd_tbl *cmdtp, int flag,
		   int argc, char *const argv[])
{
	struct cmd_tbl *misc_cmd;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;
	misc_cmd = find_cmd_tbl(argv[1], misc_commands,
				ARRAY_SIZE(misc_commands));
	argc -= 2;
	argv += 2;
	if (!misc_cmd || argc != misc_cmd->maxargs)
		return CMD_RET_USAGE;

	ret = misc_cmd->cmd(misc_cmd, flag, argc, argv);

	return cmd_process_error(misc_cmd, ret);
}

U_BOOT_CMD(
	misc,	6,	1,	do_misc,
	"Access miscellaneous devices with MISC uclass driver APIs",
	"list                       - list all miscellaneous devices\n"
	"misc read  name offset addr len - read `len' bytes starting at\n"
	"				  `offset' of device `name'\n"
	"				  to memory at `addr'\n"
	"misc write name offset addr len - write `len' bytes starting at\n"
	"				  `offset' of device `name'\n"
	"				  from memory at `addr'"
);
