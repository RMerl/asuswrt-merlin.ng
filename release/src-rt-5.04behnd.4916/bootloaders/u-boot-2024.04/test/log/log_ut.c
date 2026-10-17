// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Logging function tests.
 */

#include <common.h>
#include <console.h>
#include <log.h>
#include <test/log.h>
#include <test/suites.h>

int do_ut_log(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(log_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(log_test);

	return cmd_ut_category("log", "log_test_",
			       tests, n_ents, argc, argv);
}
