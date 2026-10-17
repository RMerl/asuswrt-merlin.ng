// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, Google Inc. All rights reserved.
 */

#include <common.h>
#include <bootstage.h>
#include <command.h>

static int do_bootstage_report(struct cmd_tbl *cmdtp, int flag, int argc,
			       char *const argv[])
{
	bootstage_report();

	return 0;
}

static int get_base_size(int argc, char *const argv[], ulong *basep,
			 ulong *sizep)
{
	char *endp;

	*basep = CONFIG_BOOTSTAGE_STASH_ADDR;
	*sizep = CONFIG_BOOTSTAGE_STASH_SIZE;
	if (argc < 2)
		return 0;
	*basep = hextoul(argv[1], &endp);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	if (argc == 2)
		return 0;
	*sizep = hextoul(argv[2], &endp);
	if (*argv[2] == 0 || *endp != 0)
		return -1;

	return 0;
}

static int do_bootstage_stash(struct cmd_tbl *cmdtp, int flag, int argc,
			      char *const argv[])
{
	ulong base, size;
	int ret;

	if (get_base_size(argc, argv, &base, &size))
		return CMD_RET_USAGE;
	if (base == -1UL) {
		printf("No bootstage stash area defined\n");
		return 1;
	}

	if (0 == strcmp(argv[0], "stash"))
		ret = bootstage_stash((void *)base, size);
	else
		ret = bootstage_unstash((void *)base, size);
	if (ret)
		return 1;

	return 0;
}

static struct cmd_tbl cmd_bootstage_sub[] = {
	U_BOOT_CMD_MKENT(report, 2, 1, do_bootstage_report, "", ""),
	U_BOOT_CMD_MKENT(stash, 4, 0, do_bootstage_stash, "", ""),
	U_BOOT_CMD_MKENT(unstash, 4, 0, do_bootstage_stash, "", ""),
};

/*
 * Process a bootstage sub-command
 */
static int do_boostage(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct cmd_tbl *c;

	/* Strip off leading 'bootstage' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_bootstage_sub,
			 ARRAY_SIZE(cmd_bootstage_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}


U_BOOT_CMD(bootstage, 4, 1, do_boostage,
	"Boot stage command",
	" - check boot progress and timing\n"
	"report                      - Print a report\n"
	"stash [<start> [<size>]]    - Stash data into memory\n"
	"unstash [<start> [<size>]]  - Unstash data from memory"
);
