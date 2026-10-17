// SPDX-License-Identifier: GPL-2.0+
/*
 * Executes tests for SEAMA (SEAttle iMAge) command
 *
 * Copyright (C) 2021 Linus Walleij <linus.walleij@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

#define SEAMA_TEST(_name, _flags)	UNIT_TEST(_name, _flags, seama_test)

static int seama_test_noargs(struct unit_test_state *uts)
{
	/* Test that 'seama' with no arguments fails gracefully */
	console_record_reset();
	run_command("seama", 0);
	ut_assert_nextlinen("seama - Load the SEAMA image and sets envs");
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_skipline();
	ut_assert_console_end();
	return 0;
}
SEAMA_TEST(seama_test_noargs, UT_TESTF_CONSOLE_REC);

static int seama_test_addr(struct unit_test_state *uts)
{
	/* Test that loads SEAMA image 0 to address 0x01000000 */
	console_record_reset();
	run_command("seama 0x01000000", 0);
	ut_assert_nextlinen("Loading SEAMA image 0 from nand0");
	ut_assert_nextlinen("SEMA IMAGE:");
	ut_assert_nextlinen("  metadata size ");
	ut_assert_nextlinen("  image size ");
	ut_assert_nextlinen("  checksum ");
	ut_assert_nextlinen("Decoding SEAMA image 0x01000040..");
	ut_assert_console_end();
	return 0;
}
SEAMA_TEST(seama_test_addr, UT_TESTF_CONSOLE_REC);

static int seama_test_index(struct unit_test_state *uts)
{
	/* Test that loads SEAMA image 0 exlicitly specified */
	console_record_reset();
	run_command("seama 0x01000000 0", 0);
	ut_assert_nextlinen("Loading SEAMA image 0 from nand0");
	ut_assert_nextlinen("SEMA IMAGE:");
	ut_assert_nextlinen("  metadata size ");
	ut_assert_nextlinen("  image size ");
	ut_assert_nextlinen("  checksum ");
	ut_assert_nextlinen("Decoding SEAMA image 0x01000040..");
	ut_assert_console_end();
	return 0;
}
SEAMA_TEST(seama_test_index, UT_TESTF_CONSOLE_REC);

int do_ut_seama(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(seama_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(seama_test);

	return cmd_ut_category("seama", "seama_test_", tests, n_ents, argc,
			       argv);
}
