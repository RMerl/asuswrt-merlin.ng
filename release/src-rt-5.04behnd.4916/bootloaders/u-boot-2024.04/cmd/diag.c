// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Diagnostics support
 */
#include <common.h>
#include <command.h>
#include <post.h>

int do_diag(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int i;

	if (argc == 1 || strcmp (argv[1], "run") != 0) {
		/* List test info */
		if (argc == 1) {
			puts ("Available hardware tests:\n");
			post_info (NULL);
			puts ("Use 'diag [<test1> [<test2> ...]]'"
					" to get more info.\n");
			puts ("Use 'diag run [<test1> [<test2> ...]]'"
					" to run tests.\n");
		} else {
			for (i = 1; i < argc; i++) {
			    if (post_info (argv[i]) != 0)
				printf ("%s - no such test\n", argv[i]);
			}
		}
	} else {
		/* Run tests */
		if (argc == 2) {
			post_run (NULL, POST_RAM | POST_MANUAL);
		} else {
			for (i = 2; i < argc; i++) {
			    if (post_run (argv[i], POST_RAM | POST_MANUAL) != 0)
				printf ("%s - unable to execute the test\n",
					argv[i]);
			}
		}
	}

	return 0;
}
/***************************************************/

U_BOOT_CMD(
	diag,	CONFIG_SYS_MAXARGS,	0,	do_diag,
	"perform board diagnostics",
	     "    - print list of available tests\n"
	"diag [test1 [test2]]\n"
	"         - print information about specified tests\n"
	"diag run - run all available tests\n"
	"diag run [test1 [test2]]\n"
	"         - run specified tests"
);
