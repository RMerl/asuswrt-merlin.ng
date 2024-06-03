// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, Google Inc.
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <spl.h>
#include <asm/state.h>

static int do_sb_handoff(cmd_tbl_t *cmdtp, int flag, int argc,
			 char *const argv[])
{
#if CONFIG_IS_ENABLED(HANDOFF)
	if (gd->spl_handoff)
		printf("SPL handoff magic %lx\n", gd->spl_handoff->arch.magic);
	else
		printf("SPL handoff info not received\n");

	return 0;
#else
	printf("Command not supported\n");

	return CMD_RET_USAGE;
#endif
}

static int do_sb_state(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct sandbox_state *state;

	state = state_get_current();
	state_show(state);

	return 0;
}

static cmd_tbl_t cmd_sb_sub[] = {
	U_BOOT_CMD_MKENT(handoff, 1, 0, do_sb_handoff, "", ""),
	U_BOOT_CMD_MKENT(state, 1, 0, do_sb_state, "", ""),
};

static int do_sb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	/* Skip past 'sb' */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_sb_sub, ARRAY_SIZE(cmd_sb_sub));
	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sb,	8,	1,	do_sb,
	"Sandbox status commands",
	"handoff     - Show handoff data received from SPL\n"
	"sb state       - Show sandbox state"
);
