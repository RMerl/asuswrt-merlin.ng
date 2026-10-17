// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 *
 * Portions of these tests were inspired by glibc's posix/bug-getopt1.c and
 * posix/tst-getopt-cancel.c
 */

#include <common.h>
#include <getopt.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int do_test_getopt(struct unit_test_state *uts, int line,
			  struct getopt_state *gs, const char *optstring,
			  int args, char *argv[], int expected_count,
			  int expected[])
{
	int opt;

	getopt_init_state(gs);
	for (int i = 0; i < expected_count; i++) {
		opt = getopt_silent(gs, args, argv, optstring);
		if (expected[i] != opt) {
			/*
			 * Fudge the line number so we can tell which test
			 * failed
			 */
			ut_failf(uts, __FILE__, line, __func__,
				 "expected[i] == getopt()",
				 "Expected '%c' (%d) with i=%d, got '%c' (%d)",
				 expected[i], expected[i], i, opt, opt);
			return CMD_RET_FAILURE;
		}
	}

	opt = getopt_silent(gs, args, argv, optstring);
	if (opt != -1) {
		ut_failf(uts, __FILE__, line, __func__,
			 "getopt() != -1",
			 "Expected -1, got '%c' (%d)", opt, opt);
		return CMD_RET_FAILURE;
	}

	return 0;
}

#define test_getopt(optstring, argv, expected) do { \
	int ret = do_test_getopt(uts, __LINE__, &gs, optstring, \
				 ARRAY_SIZE(argv) - 1, argv, \
				 ARRAY_SIZE(expected), expected); \
	if (ret) \
		return ret; \
} while (0)

static int lib_test_getopt(struct unit_test_state *uts)
{
	struct getopt_state gs;

	/* Happy path */
	test_getopt("ab:c",
		    ((char *[]){ "program", "-cb", "x", "-a", "foo", 0 }),
		    ((int []){ 'c', 'b', 'a' }));
	ut_asserteq(4, gs.index);

	/* Make sure we pick up the optional argument */
	test_getopt("a::b:c",
		    ((char *[]){ "program", "-cbx", "-a", "foo", 0 }),
		    ((int []){ 'c', 'b', 'a' }));
	ut_asserteq(4, gs.index);

	/* Test required arguments */
	test_getopt("a:b", ((char *[]){ "program", "-a", 0 }),
		    ((int []){ ':' }));
	ut_asserteq('a', gs.opt);
	test_getopt("a:b", ((char *[]){ "program", "-b", "-a", 0 }),
		    ((int []){ 'b', ':' }));
	ut_asserteq('a', gs.opt);

	/* Test invalid arguments */
	test_getopt("ab:c", ((char *[]){ "program", "-d", 0 }),
		    ((int []){ '?' }));
	ut_asserteq('d', gs.opt);

	/* Test arg */
	test_getopt("a::b:c",
		    ((char *[]){ "program", "-a", 0 }),
		    ((int []){ 'a' }));
	ut_asserteq(2, gs.index);
	ut_assertnull(gs.arg);

	test_getopt("a::b:c",
		    ((char *[]){ "program", "-afoo", 0 }),
		    ((int []){ 'a' }));
	ut_asserteq(2, gs.index);
	ut_assertnonnull(gs.arg);
	ut_asserteq_str("foo", gs.arg);

	test_getopt("a::b:c",
		    ((char *[]){ "program", "-a", "foo", 0 }),
		    ((int []){ 'a' }));
	ut_asserteq(3, gs.index);
	ut_assertnonnull(gs.arg);
	ut_asserteq_str("foo", gs.arg);

	test_getopt("a::b:c",
		    ((char *[]){ "program", "-bfoo", 0 }),
		    ((int []){ 'b' }));
	ut_asserteq(2, gs.index);
	ut_assertnonnull(gs.arg);
	ut_asserteq_str("foo", gs.arg);

	test_getopt("a::b:c",
		    ((char *[]){ "program", "-b", "foo", 0 }),
		    ((int []){ 'b' }));
	ut_asserteq(3, gs.index);
	ut_assertnonnull(gs.arg);
	ut_asserteq_str("foo", gs.arg);

	return 0;
}
LIB_TEST(lib_test_getopt, 0);
