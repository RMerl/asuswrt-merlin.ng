// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for echo command
 *
 * Copyright 2020, Heinrich Schuchadt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>
#include <asm/global_data.h>
#include <display_options.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

struct test_data {
	char *cmd;
	char *expected;
};

static struct test_data echo_data[] = {
	{"echo 1 2 3",
	 "1 2 3"},
	/* Test new line handling */
	{"echo -n 1 2 3; echo a b c",
	 "1 2 3a b c"},
	/*
	 * Test handling of environment variables.
	 *
	 * j, q, x are among the least frequent letters in English.
	 * Hence no collision for the variable name jQx is expected.
	 */
	{"setenv jQx X; echo \"a)\" ${jQx} 'b)' '${jQx}' c) ${jQx}; setenv jQx",
	 "a) X b) ${jQx} c) X"},
	/* Test shell variable assignments without substitutions */
	{"foo=bar echo baz", "baz"},
	/* Test handling of shell variables. */
	{"setenv jQx; for jQx in 1 2 3; do echo -n \"${jQx}, \"; done; echo;",
	 "1, 2, 3, "},
};

static int lib_test_hush_echo(struct unit_test_state *uts)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(echo_data); ++i) {
		ut_silence_console(uts);
		console_record_reset_enable();
		ut_assertok(run_command(echo_data[i].cmd, 0));
		ut_unsilence_console(uts);
		console_record_readline(uts->actual_str,
					sizeof(uts->actual_str));
		ut_asserteq_str(echo_data[i].expected, uts->actual_str);
		ut_assertok(ut_check_console_end(uts));
	}
	return 0;
}

LIB_TEST(lib_test_hush_echo, 0);
