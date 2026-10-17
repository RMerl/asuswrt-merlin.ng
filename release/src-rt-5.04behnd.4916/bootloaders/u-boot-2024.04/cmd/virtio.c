// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <blk.h>
#include <command.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>

static int virtio_curr_dev;

static int do_virtio(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	if (argc == 2 && !strcmp(argv[1], "scan")) {
		/*
		 * make sure all virtio devices are enumerated.
		 * Do the same as virtio_init(), but also call
		 * device_probe() for children (i.e. virtio devices)
		 */
		struct udevice *bus, *child;

		uclass_first_device(UCLASS_VIRTIO, &bus);
		if (!bus)
			return CMD_RET_FAILURE;

		while (bus) {
			device_foreach_child_probe(child, bus)
				;
			uclass_next_device(&bus);
		}

		return CMD_RET_SUCCESS;
	}

	return blk_common_cmd(argc, argv, UCLASS_VIRTIO, &virtio_curr_dev);
}

U_BOOT_CMD(
	virtio, 8, 1, do_virtio,
	"virtio block devices sub-system",
	"scan - initialize virtio bus\n"
	"virtio info - show all available virtio block devices\n"
	"virtio device [dev] - show or set current virtio block device\n"
	"virtio part [dev] - print partition table of one or all virtio block devices\n"
	"virtio read addr blk# cnt - read `cnt' blocks starting at block\n"
	"     `blk#' to memory address `addr'\n"
	"virtio write addr blk# cnt - write `cnt' blocks starting at block\n"
	"     `blk#' from memory address `addr'"
);
