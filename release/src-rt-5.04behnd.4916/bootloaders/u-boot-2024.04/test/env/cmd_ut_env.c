// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#include <common.h>
#include <command.h>
#include <test/env.h>
#include <test/suites.h>
#include <test/ut.h>

int do_ut_env(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(env_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(env_test);

	return cmd_ut_category("environment", "env_test_",
			       tests, n_ents, argc, argv);
}
