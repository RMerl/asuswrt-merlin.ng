// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Misc functions
 */
#include <common.h>
#include <command.h>
#include <console.h>

static int do_sleep(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong start = get_timer(0);
	ulong mdelay = 0;
	ulong delay;
	char *frpart;

	if (argc != 2)
		return CMD_RET_USAGE;

	delay = simple_strtoul(argv[1], NULL, 10) * CONFIG_SYS_HZ;

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
			return (-1);

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

#ifdef CONFIG_CMD_TIMER
static int do_timer(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	static ulong start;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "start"))
		start = get_timer(0);

	if (!strcmp(argv[1], "get")) {
		ulong msecs = get_timer(start) * 1000 / CONFIG_SYS_HZ;
		printf("%ld.%03d\n", msecs / 1000, (int)(msecs % 1000));
	}

	return 0;
}

U_BOOT_CMD(
	timer,    2,    1,     do_timer,
	"access the system timer",
	"start - Reset the timer reference.\n"
	"timer get   - Print the time since 'start'."
);
#endif
