// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Unit tests for command functions
 */

#include <command.h>
#include <test/cmd.h>
#include <test/suites.h>
#include <test/ut.h>

int do_ut_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(cmd_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(cmd_test);

	return cmd_ut_category("cmd", "cmd_test_", tests, n_ents, argc, argv);
}
