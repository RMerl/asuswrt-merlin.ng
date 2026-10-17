// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for loadm command
 *
 * Copyright 2022 ARM Limited
 * Copyright 2022 Linaro
 *
 * Authors:
 *   Rui Miguel Silva <rui.silva@linaro.org>
 */

#include <common.h>
#include <console.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <dm/test.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

#define BUF_SIZE 0x100

#define LOADM_TEST(_name, _flags)	UNIT_TEST(_name, _flags, loadm_test)

static int loadm_test_params(struct unit_test_state *uts)
{
	ut_assertok(console_record_reset_enable());
	run_command("loadm", 0);
	ut_assert_nextline("loadm - load binary blob from source address to destination address");

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x12345678", 0);
	ut_assert_nextline("loadm - load binary blob from source address to destination address");

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x12345678 0x12345678", 0);
	ut_assert_nextline("loadm - load binary blob from source address to destination address");

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x12345678 0x12345678 0", 0);
	ut_assert_nextline("loadm: can not load zero bytes");

	return 0;
}
LOADM_TEST(loadm_test_params, UT_TESTF_CONSOLE_REC);

static int loadm_test_load (struct unit_test_state *uts)
{
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	memset(buf, 0xaa, BUF_SIZE / 2);

	ut_assertok(console_record_reset_enable());
	run_command("loadm 0x0 0x80 0x80", 0);
	ut_assert_nextline("loaded bin to memory: size: 128");

	unmap_sysmem(buf);

	return 0;
}
LOADM_TEST(loadm_test_load, UT_TESTF_CONSOLE_REC);

int do_ut_loadm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(loadm_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(loadm_test);

	return cmd_ut_category("loadm", "loadm_test_", tests, n_ents, argc,
			       argv);
}
