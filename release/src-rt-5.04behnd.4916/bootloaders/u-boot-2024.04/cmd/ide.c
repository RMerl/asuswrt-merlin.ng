// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * IDE support
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <config.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

#include <ide.h>
#include <ata.h>

#ifdef CONFIG_LED_STATUS
# include <status_led.h>
#endif

/* Current I/O Device	*/
static int curr_device;

int do_ide(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc == 2) {
		if (strncmp(argv[1], "res", 3) == 0) {
			struct udevice *dev;
			int ret;

			puts("\nReset IDE: ");
			ret = uclass_find_first_device(UCLASS_IDE, &dev);
			ret = device_remove(dev, DM_REMOVE_NORMAL);
			if (!ret)
				ret = device_chld_unbind(dev, NULL);
			if (ret) {
				printf("Cannot remove IDE (err=%dE)\n", ret);
				return CMD_RET_FAILURE;
			}

			ret = uclass_first_device_err(UCLASS_IDE, &dev);
			if (ret) {
				printf("Init failed (err=%dE)\n", ret);
				return CMD_RET_FAILURE;
			}

			return 0;
		}
	}

	return blk_common_cmd(argc, argv, UCLASS_IDE, &curr_device);
}

int do_diskboot(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return common_diskboot(cmdtp, "ide", argc, argv);
}

U_BOOT_CMD(ide, 5, 1, do_ide,
	   "IDE sub-system",
	   "reset - reset IDE controller\n"
	   "ide info  - show available IDE devices\n"
	   "ide device [dev] - show or set current device\n"
	   "ide part [dev] - print partition table of one or all IDE devices\n"
	   "ide read  addr blk# cnt\n"
	   "ide write addr blk# cnt - read/write `cnt'"
	   " blocks starting at block `blk#'\n"
	   "    to/from memory address `addr'");

U_BOOT_CMD(diskboot, 3, 1, do_diskboot,
	   "boot from IDE device", "loadAddr dev:part");
