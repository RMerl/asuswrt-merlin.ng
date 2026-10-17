// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'conitrace' command prints the codes received from the console input as
 * hexadecimal numbers.
 *
 * Copyright (c) 2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */
#include <common.h>
#include <command.h>
#include <linux/delay.h>

static int do_conitrace(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	bool first = true;

	printf("Waiting for your input\n");
	printf("To terminate type 'x'\n");

	/* Empty input buffer */
	while (tstc())
		getchar();

	for (;;) {
		int c = getchar();

		if (first && (c == 'x' || c == 'X'))
			break;

		printf("%02x ", c);
		first = false;

		/* 10 ms delay - serves to detect separate keystrokes */
		udelay(10000);
		if (!tstc()) {
			printf("\n");
			first = true;
		}
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(conitrace, "");

U_BOOT_CMD_COMPLETE(
	conitrace, 2, 0, do_conitrace,
	"trace console input",
	conitrace_help_text, NULL
);
