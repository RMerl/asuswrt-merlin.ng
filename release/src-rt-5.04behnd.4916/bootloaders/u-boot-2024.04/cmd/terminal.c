// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Written by Harald Welte <laforge@openmoko.org>
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <stdio_dev.h>
#include <serial.h>

int do_terminal(struct cmd_tbl *cmd, int flag, int argc, char *const argv[])
{
	int last_tilde = 0;
	struct stdio_dev *dev = NULL;

	if (argc < 1)
		return -1;

	/* Scan for selected output/input device */
	dev = stdio_get_by_name(argv[1]);
	if (!dev)
		return -1;

	if (IS_ENABLED(CONFIG_SERIAL))
		serial_reinit_all();

	printf("Entering terminal mode for port %s\n", dev->name);
	puts("Use '~.' to leave the terminal and get back to u-boot\n");

	while (1) {
		int c;

		/* read from console and display on serial port */
		if (stdio_devices[0]->tstc(stdio_devices[0])) {
			c = stdio_devices[0]->getc(stdio_devices[0]);
			if (last_tilde == 1) {
				if (c == '.') {
					putc(c);
					putc('\n');
					break;
				} else {
					last_tilde = 0;
					/* write the delayed tilde */
					dev->putc(dev, '~');
					/* fall-through to print current
					 * character */
				}
			}
			if (c == '~') {
				last_tilde = 1;
				puts("[u-boot]");
				putc(c);
			}
			dev->putc(dev, c);
		}

		/* read from serial port and display on console */
		if (dev->tstc(dev)) {
			c = dev->getc(dev);
			putc(c);
		}
	}
	return 0;
}


/***************************************************/

U_BOOT_CMD(
	terminal,	3,	1,	do_terminal,
	"start terminal emulator",
	""
);
