// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 */

#include <common.h>
#include <cli.h>
#include <test/common.h>
#include <test/test.h>
#include <test/ut.h>

static int cli_ch_test(struct unit_test_state *uts)
{
	struct cli_ch_state s_cch, *cch = &s_cch;

	cli_ch_init(cch);

	/* should be nothing to return at first */
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* check normal entry */
	ut_asserteq('a', cli_ch_process(cch, 'a'));
	ut_asserteq('b', cli_ch_process(cch, 'b'));
	ut_asserteq('c', cli_ch_process(cch, 'c'));
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* send an invalid escape sequence */
	ut_asserteq(0, cli_ch_process(cch, '\e'));
	ut_asserteq(0, cli_ch_process(cch, '['));

	/*
	 * with the next char it sees that the sequence is invalid, so starts
	 * emitting it
	 */
	ut_asserteq('\e', cli_ch_process(cch, 'X'));

	/* now we set 0 bytes to empty the buffer */
	ut_asserteq('[', cli_ch_process(cch, 0));
	ut_asserteq('X', cli_ch_process(cch, 0));
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* things are normal again */
	ut_asserteq('a', cli_ch_process(cch, 'a'));
	ut_asserteq(0, cli_ch_process(cch, 0));

	/* unexpected 'Esc' */
	ut_asserteq('a', cli_ch_process(cch, 'a'));
	ut_asserteq(0, cli_ch_process(cch, '\e'));
	ut_asserteq('b', cli_ch_process(cch, 'b'));
	ut_asserteq(0, cli_ch_process(cch, 0));

	return 0;
}
COMMON_TEST(cli_ch_test, 0);

static int cread_test(struct unit_test_state *uts)
{
	int duration;
	ulong start;
	char buf[10];

	/*
	 * useful for debugging
	 *
	 * gd->flags &= ~GD_FLG_RECORD;
	 * print_buffer(0, buf, 1, 7, 0);
	 */

	console_record_reset_enable();

	/* simple input */
	*buf = '\0';
	ut_asserteq(4, console_in_puts("abc\n"));
	ut_asserteq(3, cli_readline_into_buffer("-> ", buf, 1));
	ut_asserteq_str("abc", buf);

	/* try an escape sequence (cursor left after the 'c') */
	*buf = '\0';
	ut_asserteq(8, console_in_puts("abc\e[Dx\n"));
	ut_asserteq(4, cli_readline_into_buffer("-> ", buf, 1));
	ut_asserteq_str("abxc", buf);

	/* invalid escape sequence */
	*buf = '\0';
	ut_asserteq(8, console_in_puts("abc\e[Xx\n"));
	ut_asserteq(7, cli_readline_into_buffer("-> ", buf, 1));
	ut_asserteq_str("abc\e[Xx", buf);

	/* unexpected 'Esc' */
	*buf = '\0';
	ut_asserteq(7, console_in_puts("abc\eXx\n"));
	ut_asserteq(5, cli_readline_into_buffer("-> ", buf, 1));
	ut_asserteq_str("abcXx", buf);

	/* check timeout, should be between 1000 and 1050ms */
	start = get_timer(0);
	*buf = '\0';
	ut_asserteq(-2, cli_readline_into_buffer("-> ", buf, 1));
	duration = get_timer(start) - 1000;
	ut_assert(duration >= 0);
	ut_assert(duration < 50);

	return 0;
}
COMMON_TEST(cread_test, 0);
