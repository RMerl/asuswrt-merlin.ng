// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021
 * Francis Laniel, Amarula Solutions, francis.laniel@amarulasolutions.com
 */

#include <command.h>
#include <env_attr.h>
#include <test/hush.h>
#include <test/ut.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int hush_test_for(struct unit_test_state *uts)
{
	console_record_reset_enable();

	ut_assertok(run_command("for loop_i in foo bar quux quux; do echo $loop_i; done", 0));
	ut_assert_nextline("foo");
	ut_assert_nextline("bar");
	ut_assert_nextline("quux");
	ut_assert_nextline("quux");
	ut_assert_console_end();

	if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/* Reset local variable. */
		ut_assertok(run_command("loop_i=", 0));
	} else if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		puts("Beware: this test set local variable loop_i and it cannot be unset!");
	}

	return 0;
}
HUSH_TEST(hush_test_for, 0);

static int hush_test_while(struct unit_test_state *uts)
{
	console_record_reset_enable();

	if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/*
		 * Hush 2021 always returns 0 from while loop...
		 * You can see code snippet near this line to have a better
		 * understanding:
		 * debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
		 */
		ut_assertok(run_command("while test -z \"$loop_foo\"; do echo bar; loop_foo=quux; done", 0));
	} else if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		/*
		 * Exit status is that of test, so 1 since test is false to quit
		 * the loop.
		 */
		ut_asserteq(1, run_command("while test -z \"$loop_foo\"; do echo bar; loop_foo=quux; done", 0));
	}
	ut_assert_nextline("bar");
	ut_assert_console_end();

	if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/* Reset local variable. */
		ut_assertok(run_command("loop_foo=", 0));
	} else if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		puts("Beware: this test set local variable loop_foo and it cannot be unset!");
	}

	return 0;
}
HUSH_TEST(hush_test_while, 0);

static int hush_test_until(struct unit_test_state *uts)
{
	console_record_reset_enable();
	env_set("loop_bar", "bar");

	/*
	 * WARNING We have to use environment variable because it is not possible
	 * resetting local variable.
	 */
	ut_assertok(run_command("until test -z \"$loop_bar\"; do echo quux; setenv loop_bar; done", 0));
	ut_assert_nextline("quux");
	ut_assert_console_end();

	/*
	 * Loop normally resets foo environment variable, but we reset it here in
	 * case the test failed.
	 */
	env_set("loop_bar", NULL);
	return 0;
}
HUSH_TEST(hush_test_until, 0);
