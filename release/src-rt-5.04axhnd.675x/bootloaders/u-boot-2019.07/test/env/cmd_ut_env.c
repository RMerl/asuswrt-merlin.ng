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

int do_ut_env(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, env_test);
	const int n_ents = ll_entry_count(struct unit_test, env_test);

	return cmd_ut_category("environment", tests, n_ents, argc, argv);
}
