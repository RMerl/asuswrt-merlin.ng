// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <linux/delay.h>

static int do_sleep(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	ulong start = get_timer(0);
	ulong mdelay = 0;
	ulong delay;
	char *frpart;

	if (argc != 2)
		return CMD_RET_USAGE;

	delay = dectoul(argv[1], NULL) * CONFIG_SYS_HZ;

	frpart = strchr(argv[1], '.');

	if (frpart) {
		uint mult = CONFIG_SYS_HZ / 10;
		for (frpart++; *frpart != '\0' && mult > 0; frpart++) {
			if (*frpart < '0' || *frpart > '9') {
				mdelay = 0;
				break;
			}
			mdelay += (*frpart - '0') * mult;
			mult /= 10;
		}
	}

	delay += mdelay;

	while (get_timer(start) < delay) {
		if (ctrlc())
			return CMD_RET_FAILURE;

		udelay(100);
	}

	return 0;
}

U_BOOT_CMD(
	sleep ,    2,    1,     do_sleep,
	"delay execution for some time",
	"N\n"
	"    - delay execution for N seconds (N is _decimal_ and can be\n"
	"      fractional)"
);
