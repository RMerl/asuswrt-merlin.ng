// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for pause command
 *
 * Copyright 2022, Samuel Dionne-Riel <samuel@dionne-riel.com>
 */

#include <common.h>
#include <asm/global_data.h>
#include <test/lib.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

static int lib_test_hush_pause(struct unit_test_state *uts)
{
	/* Test default message */
	console_record_reset_enable();
	/* Cook a newline when the command is expected to pause */
	console_in_puts("\n");
	ut_assertok(run_command("pause", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_str("Press any key to continue...", uts->actual_str);
	ut_assertok(ut_check_console_end(uts));

	/* Test provided message */
	console_record_reset_enable();
	/* Cook a newline when the command is expected to pause */
	console_in_puts("\n");
	ut_assertok(run_command("pause 'Prompt for pause...'", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_str("Prompt for pause...", uts->actual_str);
	ut_assertok(ut_check_console_end(uts));

	/* Test providing more than one params */
	console_record_reset_enable();
	/* No newline cooked here since the command is expected to fail */
	ut_asserteq(1, run_command("pause a b", 0));
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_str("pause - delay until user input", uts->actual_str);
	ut_asserteq(1, ut_check_console_end(uts));

	return 0;
}
LIB_TEST(lib_test_hush_pause, 0);
