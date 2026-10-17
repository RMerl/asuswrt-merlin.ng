// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <command.h>
#include <bootcount.h>

static int do_bootcount_print(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	printf("%lu\n", bootcount_load());
	return CMD_RET_SUCCESS;
}

static int do_bootcount_reset(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	/*
	 * note that we're explicitly not resetting the environment
	 * variable, so you still have the old bootcounter available
	 */
	bootcount_store(0);
	return CMD_RET_SUCCESS;
}

static struct cmd_tbl bootcount_sub[] = {
	U_BOOT_CMD_MKENT(print, 1, 1, do_bootcount_print, "", ""),
	U_BOOT_CMD_MKENT(reset, 1, 1, do_bootcount_reset, "", ""),
};

static int do_bootcount(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "bootcount" arg */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], bootcount_sub, ARRAY_SIZE(bootcount_sub));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(bootcount,
	"print - print current bootcounter\n"
	"reset - reset the bootcounter");

U_BOOT_CMD(bootcount, 2, 1, do_bootcount,
	   "bootcount",
	   bootcount_help_text
);
