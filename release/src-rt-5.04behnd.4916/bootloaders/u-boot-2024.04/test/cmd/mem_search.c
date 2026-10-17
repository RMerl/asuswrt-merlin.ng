// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for memory commands
 *
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <console.h>
#include <mapmem.h>
#include <dm/test.h>
#include <test/ut.h>

#define BUF_SIZE	0x100

/* Declare a new mem test */
#define MEM_TEST(_name, _flags)	UNIT_TEST(_name, _flags, mem_test)

/* Test 'ms' command with bytes */
static int mem_test_ms_b(struct unit_test_state *uts)
{
	u8 *buf;

	buf = map_sysmem(0, BUF_SIZE + 1);
	memset(buf, '\0', BUF_SIZE);
	buf[0x0] = 0x12;
	buf[0x31] = 0x12;
	buf[0xff] = 0x12;
	buf[0x100] = 0x12;
	ut_assertok(console_record_reset_enable());
	run_command("ms.b 1 ff 12", 0);
	ut_assert_nextline("00000030: 00 12 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................");
	ut_assert_nextline("--");
	ut_assert_nextline("000000f0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 12  ................");
	ut_assert_nextline("2 matches");
	ut_assert_console_end();

	ut_asserteq(2, env_get_hex("memmatches", 0));
	ut_asserteq(0xff, env_get_hex("memaddr", 0));
	ut_asserteq(0xfe, env_get_hex("mempos", 0));

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_b, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command with 16-bit values */
static int mem_test_ms_w(struct unit_test_state *uts)
{
	u16 *buf;

	buf = map_sysmem(0, BUF_SIZE + 2);
	memset(buf, '\0', BUF_SIZE);
	buf[0x34 / 2] = 0x1234;
	buf[BUF_SIZE / 2] = 0x1234;
	ut_assertok(console_record_reset_enable());
	run_command("ms.w 0 80 1234", 0);
	ut_assert_nextline("00000030: 0000 0000 1234 0000 0000 0000 0000 0000  ....4...........");
	ut_assert_nextline("1 match");
	ut_assert_console_end();

	ut_asserteq(1, env_get_hex("memmatches", 0));
	ut_asserteq(0x34, env_get_hex("memaddr", 0));
	ut_asserteq(0x34 / 2, env_get_hex("mempos", 0));

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_w, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command with 32-bit values */
static int mem_test_ms_l(struct unit_test_state *uts)
{
	u32 *buf;

	buf = map_sysmem(0, BUF_SIZE + 4);
	memset(buf, '\0', BUF_SIZE);
	buf[0x38 / 4] = 0x12345678;
	buf[BUF_SIZE / 4] = 0x12345678;
	ut_assertok(console_record_reset_enable());
	run_command("ms 0 40 12345678", 0);
	ut_assert_nextline("00000030: 00000000 00000000 12345678 00000000  ........xV4.....");
	ut_assert_nextline("1 match");
	ut_assert_console_end();

	ut_asserteq(1, env_get_hex("memmatches", 0));
	ut_asserteq(0x38, env_get_hex("memaddr", 0));
	ut_asserteq(0x38 / 4, env_get_hex("mempos", 0));

	ut_assertok(console_record_reset_enable());
	run_command("ms 0 80 12345679", 0);
	ut_assert_nextline("0 matches");
	ut_assert_console_end();

	ut_asserteq(0, env_get_hex("memmatches", 0));
	ut_asserteq(0, env_get_hex("memaddr", 0));
	ut_asserteq(0 / 4, env_get_hex("mempos", 0));

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_l, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command with continuation */
static int mem_test_ms_cont(struct unit_test_state *uts)
{
	char *const args[] = {"ms.b", "0", "100", "34"};
	int repeatable;
	u8 *buf;
	int i;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	for (i = 5; i < 0x33; i += 3)
		buf[i] = 0x34;
	ut_assertok(console_record_reset_enable());
	run_command("ms.b 0 100 34", 0);
	ut_assert_nextlinen("00000000: 00 00 00 00 00 34 00 00 34 00 00 34 00 00 34 00");
	ut_assert_nextline("--");
	ut_assert_nextlinen("00000010: 00 34 00 00 34 00 00 34 00 00 34 00 00 34 00 00");
	ut_assert_nextline("--");
	ut_assert_nextlinen("00000020: 34 00 00 34 00 00 34 00 00 34 00 00 34 00 00 34");
	ut_assert_nextlinen("10 matches (repeat command to check for more)");
	ut_assert_console_end();

	ut_asserteq(10, env_get_hex("memmatches", 0));
	ut_asserteq(0x20, env_get_hex("memaddr", 0));
	ut_asserteq(0x20, env_get_hex("mempos", 0));

	/*
	 * run_command() ignoes the repeatable flag when using hush, so call
	 * cmd_process() directly
	 */
	ut_assertok(console_record_reset_enable());
	cmd_process(CMD_FLAG_REPEAT, 4, args, &repeatable, NULL);
	ut_assert_nextlinen("00000020: 34 00 00 34 00 00 34 00 00 34 00 00 34 00 00 34");
	ut_assert_nextline("--");
	ut_assert_nextlinen("00000030: 00 00 34 00 00 00 00 00");
	ut_assert_nextlinen("6 matches");
	ut_assert_console_end();

	ut_asserteq(6, env_get_hex("memmatches", 0));
	ut_asserteq(0x32, env_get_hex("memaddr", 0));

	/* 0x32 less 0x21, where the second search started */
	ut_asserteq(0x11, env_get_hex("mempos", 0));

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_cont, UT_TESTF_CONSOLE_REC);

/* Test that an 'ms' command with continuation stops at the end of the range */
static int mem_test_ms_cont_end(struct unit_test_state *uts)
{
	char *const args[] = {"ms.b", "1", "ff", "12"};
	int repeatable;
	u8 *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	buf[0x0] = 0x12;
	buf[0x31] = 0x12;
	buf[0xff] = 0x12;
	buf[0x100] = 0x12;
	ut_assertok(console_record_reset_enable());
	run_command("ms.b 1 ff 12", 0);
	ut_assert_nextlinen("00000030");
	ut_assert_nextlinen("--");
	ut_assert_nextlinen("000000f0");
	ut_assert_nextlinen("2 matches");
	ut_assert_console_end();

	/*
	 * run_command() ignoes the repeatable flag when using hush, so call
	 * cmd_process() directly.
	 *
	 * This should produce no matches.
	 */
	ut_assertok(console_record_reset_enable());
	cmd_process(CMD_FLAG_REPEAT, 4, args, &repeatable, NULL);
	ut_assert_nextlinen("0 matches");
	ut_assert_console_end();

	/* One more time */
	ut_assertok(console_record_reset_enable());
	cmd_process(CMD_FLAG_REPEAT, 4, args, &repeatable, NULL);
	ut_assert_nextlinen("0 matches");
	ut_assert_console_end();

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_cont_end, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command with multiple values */
static int mem_test_ms_mult(struct unit_test_state *uts)
{
	static const char str[] = "hello";
	char *buf;

	buf = map_sysmem(0, BUF_SIZE + 5);
	memset(buf, '\0', BUF_SIZE);
	strcpy(buf + 0x1e, str);
	strcpy(buf + 0x63, str);
	strcpy(buf + BUF_SIZE - strlen(str) + 1, str);
	ut_assertok(console_record_reset_enable());
	run_command("ms.b 0 100 68 65 6c 6c 6f", 0);
	ut_assert_nextline("00000010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 68 65  ..............he");
	ut_assert_nextline("00000020: 6c 6c 6f 00 00 00 00 00 00 00 00 00 00 00 00 00  llo.............");
	ut_assert_nextline("--");
	ut_assert_nextline("00000060: 00 00 00 68 65 6c 6c 6f 00 00 00 00 00 00 00 00  ...hello........");
	ut_assert_nextline("2 matches");
	ut_assert_console_end();
	unmap_sysmem(buf);

	ut_asserteq(2, env_get_hex("memmatches", 0));
	ut_asserteq(0x63, env_get_hex("memaddr", 0));
	ut_asserteq(0x63, env_get_hex("mempos", 0));

	return 0;
}
MEM_TEST(mem_test_ms_mult, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command with string */
static int mem_test_ms_s(struct unit_test_state *uts)
{
	static const char str[] = "hello";
	static const char str2[] = "hellothere";
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\0', BUF_SIZE);
	strcpy(buf + 0x1e, str);
	strcpy(buf + 0x63, str);
	strcpy(buf + 0xa1, str2);
	ut_assertok(console_record_reset_enable());
	run_command("ms.s 0 100 hello", 0);
	ut_assert_nextline("00000010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 68 65  ..............he");
	ut_assert_nextline("00000020: 6c 6c 6f 00 00 00 00 00 00 00 00 00 00 00 00 00  llo.............");
	ut_assert_nextline("--");
	ut_assert_nextline("00000060: 00 00 00 68 65 6c 6c 6f 00 00 00 00 00 00 00 00  ...hello........");
	ut_assert_nextline("--");
	ut_assert_nextline("000000a0: 00 68 65 6c 6c 6f 74 68 65 72 65 00 00 00 00 00  .hellothere.....");
	ut_assert_nextline("3 matches");
	ut_assert_console_end();

	ut_asserteq(3, env_get_hex("memmatches", 0));
	ut_asserteq(0xa1, env_get_hex("memaddr", 0));
	ut_asserteq(0xa1, env_get_hex("mempos", 0));

	ut_assertok(console_record_reset_enable());
	run_command("ms.s 0 100 hello there", 0);
	ut_assert_nextline("000000a0: 00 68 65 6c 6c 6f 74 68 65 72 65 00 00 00 00 00  .hellothere.....");
	ut_assert_nextline("1 match");
	ut_assert_console_end();

	ut_asserteq(1, env_get_hex("memmatches", 0));
	ut_asserteq(0xa1, env_get_hex("memaddr", 0));
	ut_asserteq(0xa1, env_get_hex("mempos", 0));

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_s, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command with limit */
static int mem_test_ms_limit(struct unit_test_state *uts)
{
	u8 *buf;

	buf = map_sysmem(0, BUF_SIZE + 1);
	memset(buf, '\0', BUF_SIZE);
	buf[0x0] = 0x12;
	buf[0x31] = 0x12;
	buf[0x62] = 0x12;
	buf[0x76] = 0x12;
	ut_assertok(console_record_reset_enable());
	run_command("ms.b -l2 1 ff 12", 0);
	ut_assert_nextline("00000030: 00 12 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................");
	ut_assert_nextline("--");
	ut_assert_nextlinen("00000060: 00 00 12 00 00 00 00 00 00 00 00 00 00 00 00 00");
	ut_assert_nextline("2 matches (repeat command to check for more)");
	ut_assert_console_end();

	ut_asserteq(2, env_get_hex("memmatches", 0));
	ut_asserteq(0x62, env_get_hex("memaddr", 0));
	ut_asserteq(0x61, env_get_hex("mempos", 0));

	unmap_sysmem(buf);

	return 0;
}
MEM_TEST(mem_test_ms_limit, UT_TESTF_CONSOLE_REC);

/* Test 'ms' command in quiet mode */
static int mem_test_ms_quiet(struct unit_test_state *uts)
{
	u8 *buf;

	buf = map_sysmem(0, BUF_SIZE + 1);
	memset(buf, '\0', BUF_SIZE);
	buf[0x0] = 0x12;
	buf[0x31] = 0x12;
	buf[0x62] = 0x12;
	buf[0x76] = 0x12;
	ut_assertok(console_record_reset_enable());
	run_command("ms.b -q -l2 1 ff 12", 0);
	ut_assert_console_end();
	unmap_sysmem(buf);

	ut_asserteq(2, env_get_hex("memmatches", 0));
	ut_asserteq(0x62, env_get_hex("memaddr", 0));
	ut_asserteq(0x61, env_get_hex("mempos", 0));

	return 0;
}
MEM_TEST(mem_test_ms_quiet, UT_TESTF_CONSOLE_REC);
