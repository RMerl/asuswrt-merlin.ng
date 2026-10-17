// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for addrmap command
 *
 * Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <console.h>
#include <test/suites.h>
#include <test/ut.h>

/* Declare a new addrmap test */
#define ADDRMAP_TEST(_name, _flags)	UNIT_TEST(_name, _flags, addrmap_test)

/* Test 'addrmap' command output */
static int addrmap_test_basic(struct unit_test_state *uts)
{
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("addrmap", 0));
	ut_assert_nextline("           vaddr            paddr             size");
	ut_assert_nextline("================ ================ ================");
	/* There should be at least one entry */
	ut_assertok(!ut_check_console_end(uts));

	return 0;
}
ADDRMAP_TEST(addrmap_test_basic, UT_TESTF_CONSOLE_REC);

int do_ut_addrmap(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(addrmap_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(addrmap_test);

	return cmd_ut_category("cmd_addrmap", "cmd_addrmap_", tests, n_ents,
			       argc, argv);
}
