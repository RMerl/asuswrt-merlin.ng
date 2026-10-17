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

static int hush_test_semicolon(struct unit_test_state *uts)
{
	/* A; B = B truth table. */
	ut_asserteq(1, run_command("false; false", 0));
	ut_assertok(run_command("false; true", 0));
	ut_assertok(run_command("true; true", 0));
	ut_asserteq(1, run_command("true; false", 0));

	return 0;
}
HUSH_TEST(hush_test_semicolon, 0);

static int hush_test_and(struct unit_test_state *uts)
{
	/* A && B truth table. */
	ut_asserteq(1, run_command("false && false", 0));
	ut_asserteq(1, run_command("false && true", 0));
	ut_assertok(run_command("true && true", 0));
	ut_asserteq(1, run_command("true && false", 0));

	return 0;
}
HUSH_TEST(hush_test_and, 0);

static int hush_test_or(struct unit_test_state *uts)
{
	/* A || B truth table. */
	ut_asserteq(1, run_command("false || false", 0));
	ut_assertok(run_command("false || true", 0));
	ut_assertok(run_command("true || true", 0));
	ut_assertok(run_command("true || false", 0));

	return 0;
}
HUSH_TEST(hush_test_or, 0);

DECLARE_GLOBAL_DATA_PTR;

static int hush_test_and_or(struct unit_test_state *uts)
{
	/* A && B || C truth table. */
	ut_asserteq(1, run_command("false && false || false", 0));

	if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		ut_asserteq(1, run_command("false && false || true", 0));
	} else if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/*
		 * This difference seems to come from a bug solved in Busybox
		 * hush.
		 *
		 * Indeed, the following expression can be seen like this:
		 * (false && false) || true
		 * So, (false && false) returns 1, the second false is not
		 * executed, and true is executed because of ||.
		 */
		ut_assertok(run_command("false && false || true", 0));
	}

	if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		ut_asserteq(1, run_command("false && true || true", 0));
	} else if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/*
		 * This difference seems to come from a bug solved in Busybox
		 * hush.
		 *
		 * Indeed, the following expression can be seen like this:
		 * (false && true) || true
		 * So, (false && true) returns 1, the true is not executed, and
		 * true is executed because of ||.
		 */
		ut_assertok(run_command("false && true || true", 0));
	}

	ut_asserteq(1, run_command("false && true || false", 0));
	ut_assertok(run_command("true && true || false", 0));
	ut_asserteq(1, run_command("true && false || false", 0));
	ut_assertok(run_command("true && false || true", 0));
	ut_assertok(run_command("true && true || true", 0));

	return 0;
}
HUSH_TEST(hush_test_and_or, 0);

static int hush_test_or_and(struct unit_test_state *uts)
{
	/* A || B && C truth table. */
	ut_asserteq(1, run_command("false || false && false", 0));
	ut_asserteq(1, run_command("false || false && true", 0));
	ut_assertok(run_command("false || true && true", 0));
	ut_asserteq(1, run_command("false || true && false", 0));

	if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		ut_assertok(run_command("true || true && false", 0));
	} else if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/*
		 * This difference seems to come from a bug solved in Busybox
		 * hush.
		 *
		 * Indeed, the following expression can be seen like this:
		 * (true || true) && false
		 * So, (true || true) returns 0, the second true is not
		 * executed, and then false is executed because of &&.
		 */
		ut_asserteq(1, run_command("true || true && false", 0));
	}

	if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		ut_assertok(run_command("true || false && false", 0));
	} else if (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		/*
		 * This difference seems to come from a bug solved in Busybox
		 * hush.
		 *
		 * Indeed, the following expression can be seen like this:
		 * (true || false) && false
		 * So, (true || false) returns 0, the false is not executed, and
		 * then false is executed because of &&.
		 */
		ut_asserteq(1, run_command("true || false && false", 0));
	}

	ut_assertok(run_command("true || false && true", 0));
	ut_assertok(run_command("true || true && true", 0));

	return 0;
}
HUSH_TEST(hush_test_or_and, 0);
