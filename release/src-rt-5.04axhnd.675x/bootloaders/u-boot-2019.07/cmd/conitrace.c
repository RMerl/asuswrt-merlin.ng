// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'conitrace' command prints the codes received from the console input as
 * hexadecimal numbers.
 *
 * Copyright (c) 2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */
#include <common.h>
#include <command.h>

static int do_conitrace(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	bool first = true;

	printf("Waiting for your input\n");
	printf("To terminate type 'x'\n");

	/* Empty input buffer */
	while (tstc())
		getc();

	for (;;) {
		int c = getc();

		if (first && (c == 'x' || c == 'X'))
			break;

		printf("%02x ", c);
		first = false;

		/* 1 ms delay - serves to detect separate keystrokes */
		udelay(1000);
		if (!tstc()) {
			printf("\n");
			first = true;
		}
	}

	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_SYS_LONGHELP
static char conitrace_help_text[] = "";
#endif

U_BOOT_CMD_COMPLETE(
	conitrace, 2, 0, do_conitrace,
	"trace console input",
	conitrace_help_text, NULL
);
