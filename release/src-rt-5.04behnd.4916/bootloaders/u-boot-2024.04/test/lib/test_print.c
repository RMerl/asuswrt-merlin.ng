// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for print functions
 *
 * Copyright 2020, Heinrich Schuchadt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>
#include <display_options.h>
#include <asm/global_data.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

static int test_print_freq(struct unit_test_state *uts,
			   uint64_t freq, char *expected)
{
	ut_silence_console(uts);
	console_record_reset_enable();
	print_freq(freq, ";\n");
	ut_unsilence_console(uts);
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_str(expected, uts->actual_str);
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int lib_test_print_freq(struct unit_test_state *uts)
{
	ut_assertok(test_print_freq(uts, 321, "321 Hz;"));
	ut_assertok(test_print_freq(uts, 4321, "4.32 kHz;"));
	ut_assertok(test_print_freq(uts, 54321, "54.32 kHz;"));
	ut_assertok(test_print_freq(uts, 654321, "654.32 kHz;"));
	ut_assertok(test_print_freq(uts, 7654321, "7.66 MHz;"));
	ut_assertok(test_print_freq(uts, 87654321, "87.66 MHz;"));
	ut_assertok(test_print_freq(uts, 987654321, "987.66 MHz;"));
	ut_assertok(test_print_freq(uts, 1987654321, "1.99 GHz;"));
	ut_assertok(test_print_freq(uts, 54321987654321, "54321.99 GHz;"));
	return 0;
}

LIB_TEST(lib_test_print_freq, 0);

static int test_print_size(struct unit_test_state *uts,
			   uint64_t freq, char *expected)
{
	ut_silence_console(uts);
	console_record_reset_enable();
	print_size(freq, ";\n");
	ut_unsilence_console(uts);
	console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	ut_asserteq_str(expected, uts->actual_str);
	ut_assertok(ut_check_console_end(uts));

	return 0;
}

static int lib_test_print_size(struct unit_test_state *uts)
{
	ut_assertok(test_print_size(uts, 321, "321 Bytes;"));
	ut_assertok(test_print_size(uts, 4321, "4.2 KiB;"));
	ut_assertok(test_print_size(uts, 54321, "53 KiB;"));
	ut_assertok(test_print_size(uts, 654321, "639 KiB;"));
	ut_assertok(test_print_size(uts, 7654321, "7.3 MiB;"));
	ut_assertok(test_print_size(uts, 87654321, "83.6 MiB;"));
	ut_assertok(test_print_size(uts, 987654321, "941.9 MiB;"));
	ut_assertok(test_print_size(uts, 1073689395, "1023.9 MiB;"));
	ut_assertok(test_print_size(uts, 1073689396, "1 GiB;"));
	ut_assertok(test_print_size(uts, 1073741824, "1 GiB;"));
	ut_assertok(test_print_size(uts, 1987654321, "1.9 GiB;"));
	ut_assertok(test_print_size(uts, 54321987654321, "49.4 TiB;"));
	return 0;
}

LIB_TEST(lib_test_print_size, 0);
