// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <test/suites.h>
#include <test/test.h>

int do_ut_dm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(dm_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(dm_test);

	return cmd_ut_category("driver model", "dm_test_", tests, n_ents, argc,
			       argv);
}
