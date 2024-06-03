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

int do_ut_lib(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, lib_test);
	const int n_ents = ll_entry_count(struct unit_test, lib_test);

	return cmd_ut_category("lib", tests, n_ents, argc, argv);
}
