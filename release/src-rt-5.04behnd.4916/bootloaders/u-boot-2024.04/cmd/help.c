// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>

static int do_help(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const int len = ll_entry_count(struct cmd_tbl, cmd);
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

#ifdef CONFIG_CMDLINE
/*
 * This does not use the U_BOOT_CMD macro as ? can't be used in symbol names
 * nor can we rely on the CONFIG_SYS_LONGHELP helper macro
 */
ll_entry_declare(struct cmd_tbl, question_mark, cmd) = {
	"?",	CONFIG_SYS_MAXARGS, cmd_always_repeatable,	do_help,
	"alias for 'help'",
#ifdef  CONFIG_SYS_LONGHELP
	""
#endif /* CONFIG_SYS_LONGHELP */
};
#endif
