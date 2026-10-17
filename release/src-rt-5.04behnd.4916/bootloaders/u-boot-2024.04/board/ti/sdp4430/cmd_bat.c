// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Texas Instruments
 */

#include <common.h>
#include <command.h>

#ifdef CONFIG_CMD_BAT
#include <twl6030.h>

int do_vbat(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc == 2) {
		if (strncmp(argv[1], "startcharge", 12) == 0)
			twl6030_start_usb_charging();
		else if (strncmp(argv[1], "stopcharge", 11) == 0)
			twl6030_stop_usb_charging();
		else if (strncmp(argv[1], "status", 7) == 0) {
			twl6030_get_battery_voltage();
			twl6030_get_battery_current();
		} else {
			goto bat_cmd_usage;
		}
	} else {
		goto bat_cmd_usage;
	}
	return 0;

bat_cmd_usage:
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	bat, 2, 1, do_vbat,
	"battery charging, voltage/current measurements",
	"status - display battery voltage and current\n"
	"bat startcharge - start charging via USB\n"
	"bat stopcharge - stop charging\n"
);
#endif /* CONFIG_CMD_BAT */
