// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog commands
 *
 * Copyright (c) 2019 Michael Walle <michael@walle.cc>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <wdt.h>

static struct udevice *currdev;

static int do_wdt_list(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_WDT, &uc);
	if (ret)
		return CMD_RET_FAILURE;

	uclass_foreach_dev(dev, uc)
		printf("%s (%s)\n", dev->name, dev->driver->name);

	return CMD_RET_SUCCESS;
}

static int do_wdt_dev(cmd_tbl_t *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int ret;

	if (argc > 1) {
		ret = uclass_get_device_by_name(UCLASS_WDT, argv[1], &currdev);
		if (ret) {
			printf("Can't get the watchdog timer: %s\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	} else {
		if (!currdev) {
			printf("No watchdog timer device set!\n");
			return CMD_RET_FAILURE;
		}
		printf("dev: %s\n", currdev->name);
	}

	return CMD_RET_SUCCESS;
}

static int check_currdev(void)
{
	if (!currdev) {
		printf("No device set, use 'wdt dev' first\n");
		return CMD_RET_FAILURE;
	}
	return 0;
}

static int do_wdt_start(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	u64 timeout;
	ulong flags = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	ret = check_currdev();
	if (ret)
		return ret;

	timeout = simple_strtoull(argv[1], NULL, 0);
	if (argc > 2)
		flags = simple_strtoul(argv[2], NULL, 0);

	ret = wdt_start(currdev, timeout, flags);
	if (ret == -ENOSYS) {
		printf("Starting watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Starting watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_wdt_stop(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	ret = check_currdev();
	if (ret)
		return ret;

	ret = wdt_stop(currdev);
	if (ret == -ENOSYS) {
		printf("Stopping watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Stopping watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_wdt_reset(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;

	ret = check_currdev();
	if (ret)
		return ret;

	ret = wdt_reset(currdev);
	if (ret == -ENOSYS) {
		printf("Resetting watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Resetting watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_wdt_expire(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ret;
	ulong flags = 0;

	ret = check_currdev();
	if (ret)
		return ret;

	if (argc > 1)
		flags = simple_strtoul(argv[1], NULL, 0);

	ret = wdt_expire_now(currdev, flags);
	if (ret == -ENOSYS) {
		printf("Expiring watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Expiring watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static char wdt_help_text[] =
	"list - list watchdog devices\n"
	"wdt dev [<name>] - get/set current watchdog device\n"
	"wdt start <timeout ms> [flags] - start watchdog timer\n"
	"wdt stop - stop watchdog timer\n"
	"wdt reset - reset watchdog timer\n"
	"wdt expire [flags] - expire watchdog timer immediately\n";

U_BOOT_CMD_WITH_SUBCMDS(wdt, "Watchdog sub-system", wdt_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_wdt_list),
	U_BOOT_SUBCMD_MKENT(dev, 2, 1, do_wdt_dev),
	U_BOOT_SUBCMD_MKENT(start, 3, 1, do_wdt_start),
	U_BOOT_SUBCMD_MKENT(stop, 1, 1, do_wdt_stop),
	U_BOOT_SUBCMD_MKENT(reset, 1, 1, do_wdt_reset),
	U_BOOT_SUBCMD_MKENT(expire, 2, 1, do_wdt_expire));
