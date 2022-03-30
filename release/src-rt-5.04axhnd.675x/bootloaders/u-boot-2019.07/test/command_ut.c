// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012, The Chromium Authors
 */

#define DEBUG

#include <common.h>

static const char test_cmd[] = "setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3\0"
		"setenv list ${list}4";

static int do_ut_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s: Testing commands\n", __func__);
	run_command("env default -f -a", 0);

	/* commands separated by \n */
	run_command_list("setenv list 1\n setenv list ${list}1", -1, 0);
	assert(!strcmp("11", env_get("list")));

	/* command followed by \n and nothing else */
	run_command_list("setenv list 1${list}\n", -1, 0);
	assert(!strcmp("111", env_get("list")));

	/* a command string with \0 in it. Stuff after \0 should be ignored */
	run_command("setenv list", 0);
	run_command_list(test_cmd, sizeof(test_cmd), 0);
	assert(!strcmp("123", env_get("list")));

	/*
	 * a command list where we limit execution to only the first command
	 * using the length parameter.
	 */
	run_command_list("setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3", strlen("setenv list 1"), 0);
	assert(!strcmp("1", env_get("list")));

	assert(run_command("false", 0) == 1);
	assert(run_command("echo", 0) == 0);
	assert(run_command_list("false", -1, 0) == 1);
	assert(run_command_list("echo", -1, 0) == 0);

#ifdef CONFIG_HUSH_PARSER
	run_command("setenv foo 'setenv black 1\nsetenv adder 2'", 0);
	run_command("run foo", 0);
	assert(env_get("black") != NULL);
	assert(!strcmp("1", env_get("black")));
	assert(env_get("adder") != NULL);
	assert(!strcmp("2", env_get("adder")));
#endif

	assert(run_command("", 0) == 0);
	assert(run_command(" ", 0) == 0);

	assert(run_command("'", 0) == 1);

	printf("%s: Everything went swimmingly\n", __func__);
	return 0;
}

U_BOOT_CMD(
	ut_cmd,	5,	1,	do_ut_cmd,
	"Very basic test of command parsers",
	""
);
