// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for history command
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cli.h>
#include <command.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int lib_test_history(struct unit_test_state *uts)
{
	static const char cmd1[] = "setenv fred hello";
	static const char cmd2[] = "print fred";

	/* running commands directly does not add to history */
	ut_assertok(run_command(cmd1, 0));
	ut_assert_console_end();
	ut_assertok(run_command("history", 0));
	ut_assert_console_end();

	/* enter commands via the console */
	console_in_puts(cmd1);
	console_in_puts("\n");
	ut_asserteq(strlen(cmd1), cli_readline(""));
	ut_assert_nextline(cmd1);

	console_in_puts(cmd2);
	console_in_puts("\n");
	ut_asserteq(strlen(cmd2), cli_readline(""));
	ut_assert_nextline(cmd2);

	ut_assertok(run_command("print fred", 0));
	ut_assert_nextline("fred=hello");
	ut_assert_console_end();

	ut_assertok(run_command("history", 0));
	ut_assert_nextline(cmd1);
	ut_assert_nextline(cmd2);
	ut_assert_console_end();

	return 0;
}
LIB_TEST(lib_test_history, UT_TESTF_CONSOLE_REC);
