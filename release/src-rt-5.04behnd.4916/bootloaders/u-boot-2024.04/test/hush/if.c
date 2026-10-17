// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021
 * Francis Laniel, Amarula Solutions, francis.laniel@amarulasolutions.com
 */

#include <command.h>
#include <env_attr.h>
#include <vsprintf.h>
#include <test/hush.h>
#include <test/ut.h>

/*
 * All tests will execute the following:
 * if condition_to_test; then
 *   true
 * else
 *   false
 * fi
 * If condition is true, command returns 1, 0 otherwise.
 */
const char *if_format = "if %s; then true; else false; fi";

static int hush_test_if_base(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "true");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "false");
	ut_asserteq(1, run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_base, 0);

static int hush_test_if_basic_operators(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test aaa = aaa");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa = bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa != bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa != aaa");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa < bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test bbb < aaa");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test bbb > aaa");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa > bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -eq 123");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -eq 456");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -ne 456");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -ne 123");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -lt 456");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -lt 123");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 456 -lt 123");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -le 456");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -le 123");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 456 -le 123");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 456 -gt 123");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -gt 123");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -gt 456");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 456 -ge 123");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -ge 123");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 123 -ge 456");
	ut_asserteq(1, run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_basic_operators, 0);

static int hush_test_if_octal(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test 010 -eq 010");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 010 -eq 011");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 010 -ne 011");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 010 -ne 010");
	ut_asserteq(1, run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_octal, 0);

static int hush_test_if_hexadecimal(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test 0x2000000 -gt 0x2000001");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0x2000000 -gt 0x2000000");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0x2000000 -gt 0x1ffffff");
	ut_assertok(run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_hexadecimal, 0);

static int hush_test_if_mixed(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test 010 -eq 10");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 010 -ne 10");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0xa -eq 10");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0xa -eq 012");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 2000000 -gt 0x1ffffff");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0x2000000 -gt 1ffffff");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0x2000000 -lt 1ffffff");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0x2000000 -eq 2000000");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test 0x2000000 -ne 2000000");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test -z \"\"");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test -z \"aaa\"");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test -n \"aaa\"");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test -n \"\"");
	ut_asserteq(1, run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_mixed, 0);

static int hush_test_if_inverted(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test ! aaa = aaa");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! aaa = bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! ! aaa = aaa");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! ! aaa = bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_inverted, 0);

static int hush_test_if_binary(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test aaa != aaa -o bbb != bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa != aaa -o bbb = bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa = aaa -o bbb != bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa = aaa -o bbb = bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa != aaa -a bbb != bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa != aaa -a bbb = bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa = aaa -a bbb != bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test aaa = aaa -a bbb = bbb");
	ut_assertok(run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_binary, 0);

static int hush_test_if_inverted_binary(struct unit_test_state *uts)
{
	char if_formatted[128];

	sprintf(if_formatted, if_format, "test ! aaa != aaa -o ! bbb != bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! aaa != aaa -o ! bbb = bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! aaa = aaa -o ! bbb != bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! aaa = aaa -o ! bbb = bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format,
		"test ! ! aaa != aaa -o ! ! bbb != bbb");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format,
		"test ! ! aaa != aaa -o ! ! bbb = bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format,
		"test ! ! aaa = aaa -o ! ! bbb != bbb");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test ! ! aaa = aaa -o ! ! bbb = bbb");
	ut_assertok(run_command(if_formatted, 0));

	return 0;
}
HUSH_TEST(hush_test_if_inverted_binary, 0);

static int hush_test_if_z_operator(struct unit_test_state *uts)
{
	char if_formatted[128];

	/* Deal with environment variable used during test. */
	env_set("ut_var_nonexistent", NULL);
	env_set("ut_var_exists", "1");
	env_set("ut_var_unset", "1");

	sprintf(if_formatted, if_format, "test -z \"$ut_var_nonexistent\"");
	ut_assertok(run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test -z \"$ut_var_exists\"");
	ut_asserteq(1, run_command(if_formatted, 0));

	sprintf(if_formatted, if_format, "test -z \"$ut_var_unset\"");
	ut_asserteq(1, run_command(if_formatted, 0));

	env_set("ut_var_unset", NULL);
	sprintf(if_formatted, if_format, "test -z \"$ut_var_unset\"");
	ut_assertok(run_command(if_formatted, 0));

	/* Clear the set environment variable. */
	env_set("ut_var_exists", NULL);

	return 0;
}
HUSH_TEST(hush_test_if_z_operator, 0);
