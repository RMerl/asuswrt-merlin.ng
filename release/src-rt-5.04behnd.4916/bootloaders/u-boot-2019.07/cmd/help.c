// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>

static int do_help(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_CMDLINE
	cmd_tbl_t *start = ll_entry_start(cmd_tbl_t, cmd);
	const int len = ll_entry_count(cmd_tbl_t, cmd);
	return _do_help(start, len, cmdtp, flag, argc, argv);
#else
	return 0;
#endif
}

U_BOOT_CMD(
	help,	CONFIG_SYS_MAXARGS,	1,	do_help,
	"print command description/usage",
	"\n"
	"	- print brief description of all commands\n"
	"help command ...\n"
	"	- print detailed usage of 'command'"
);

/* This does not use the U_BOOT_CMD macro as ? can't be used in symbol names */
ll_entry_declare(cmd_tbl_t, question_mark, cmd) = {
	"?",	CONFIG_SYS_MAXARGS, cmd_always_repeatable,	do_help,
	"alias for 'help'",
#ifdef  CONFIG_SYS_LONGHELP
	""
#endif /* CONFIG_SYS_LONGHELP */
};
