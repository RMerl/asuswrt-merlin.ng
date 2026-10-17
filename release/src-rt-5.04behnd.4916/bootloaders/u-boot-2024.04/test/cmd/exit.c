// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for exit command
 *
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <console.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Declare a new exit test */
#define EXIT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, exit_test)

/* Test 'exit addr' getting/setting address */
static int cmd_exit_test(struct unit_test_state *uts)
{
	int i;

	/*
	 * Test 'exit' with parameter -3, -2, -1, 0, 1, 2, 3 . Use all those
	 * parameters to cover also the special return value -2 that is used
	 * in HUSH to detect exit command.
	 *
	 * Always test whether 'exit' command:
	 * - exits out of the 'run' command
	 * - return value is propagated out of the 'run' command
	 * - return value can be tested on outside of 'run' command
	 * - return value can be printed outside of 'run' command
	 */
	for (i = -3; i <= 3; i++) {
		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("setenv foo 'echo bar ; exit %d ; echo baz' ; run foo ; echo $?", i));
		ut_assert_nextline("bar");
		ut_assert_nextline("%d", i > 0 ? i : 0);
		ut_assertok(ut_check_console_end(uts));

		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("setenv foo 'echo bar ; exit %d ; echo baz' ; run foo && echo quux ; echo $?", i));
		ut_assert_nextline("bar");
		if (i <= 0)
			ut_assert_nextline("quux");
		ut_assert_nextline("%d", i > 0 ? i : 0);
		ut_assertok(ut_check_console_end(uts));

		ut_assertok(console_record_reset_enable());
		ut_assertok(run_commandf("setenv foo 'echo bar ; exit %d ; echo baz' ; run foo || echo quux ; echo $?", i));
		ut_assert_nextline("bar");
		if (i > 0)
			ut_assert_nextline("quux");
		/* Either 'exit' returns 0, or 'echo quux' returns 0 */
		ut_assert_nextline("0");
		ut_assertok(ut_check_console_end(uts));
	}

	/* Validate that 'exit' behaves the same way as 'exit 0' */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; exit ; echo baz' ; run foo ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; exit ; echo baz' ; run foo && echo quux ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("quux");
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; exit ; echo baz' ; run foo || echo quux ; echo $?"));
	ut_assert_nextline("bar");
	/* Either 'exit' returns 0, or 'echo quux' returns 0 */
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	/* Validate that return value still propagates from 'run' command */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; true' ; run foo ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; true' ; run foo && echo quux ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("quux");
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; true' ; run foo || echo quux ; echo $?"));
	ut_assert_nextline("bar");
	/* The 'true' returns 0 */
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; false' ; run foo ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("1");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; false' ; run foo && echo quux ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("1");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("setenv foo 'echo bar ; false' ; run foo || echo quux ; echo $?"));
	ut_assert_nextline("bar");
	ut_assert_nextline("quux");
	/* The 'echo quux' returns 0 */
	ut_assert_nextline("0");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

EXIT_TEST(cmd_exit_test, UT_TESTF_CONSOLE_REC);

int do_ut_exit(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(exit_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(exit_test);

	return cmd_ut_category("cmd_exit", "exit_test_", tests, n_ents,
			       argc, argv);
}
