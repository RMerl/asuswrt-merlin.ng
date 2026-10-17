// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Unit tests for library functions
 */

#include <common.h>
#include <command.h>
#include <test/lib.h>
#include <test/suites.h>
#include <test/ut.h>

int do_ut_lib(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(lib_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(lib_test);

	return cmd_ut_category("lib", "lib_test_", tests, n_ents, argc, argv);
}
