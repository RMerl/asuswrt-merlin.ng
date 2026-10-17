// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2021
 * Francis Laniel, Amarula Solutions, francis.laniel@amarulasolutions.com
 */

#include <command.h>
#include <test/hush.h>
#include <test/suites.h>
#include <test/ut.h>

int do_ut_hush(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(hush_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(hush_test);

	return cmd_ut_category("hush", "hush_test_",
			       tests, n_ents, argc, argv);
}
