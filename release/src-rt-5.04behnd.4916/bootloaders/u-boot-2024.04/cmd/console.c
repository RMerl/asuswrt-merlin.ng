// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <iomux.h>
#include <stdio_dev.h>

extern void _do_coninfo (void);
static int do_coninfo(struct cmd_tbl *cmd, int flag, int argc,
		      char *const argv[])
{
	int l;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;

	/* Scan for valid output and input devices */

	puts("List of available devices\n");

	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		printf("|-- %s (%s%s)\n",
		       dev->name,
		       (dev->flags & DEV_FLAGS_INPUT) ? "I" : "",
		       (dev->flags & DEV_FLAGS_OUTPUT) ? "O" : "");

		for (l = 0; l < MAX_FILES; l++) {
			if (CONFIG_IS_ENABLED(CONSOLE_MUX)) {
				if (iomux_match_device(console_devices[l],
						       cd_count[l], dev) >= 0)
					printf("|   |-- %s\n", stdio_names[l]);
			} else {
				if (stdio_devices[l] == dev)
					printf("|   |-- %s\n", stdio_names[l]);
			}

		}
	}
	return 0;
}


/***************************************************/

U_BOOT_CMD(
	coninfo,	3,	1,	do_coninfo,
	"print console devices and information",
	""
);
