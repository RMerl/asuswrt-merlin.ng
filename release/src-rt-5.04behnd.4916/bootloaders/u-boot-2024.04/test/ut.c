// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple unit test library
 *
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <console.h>
#include <malloc.h>
#ifdef CONFIG_SANDBOX
#include <asm/state.h>
#endif
#include <asm/global_data.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

void ut_fail(struct unit_test_state *uts, const char *fname, int line,
	     const char *func, const char *cond)
{
	gd->flags &= ~(GD_FLG_SILENT | GD_FLG_RECORD);
	printf("%s:%d, %s(): %s\n", fname, line, func, cond);
	uts->fail_count++;
}

void ut_failf(struct unit_test_state *uts, const char *fname, int line,
	      const char *func, const char *cond, const char *fmt, ...)
{
	va_list args;

	gd->flags &= ~(GD_FLG_SILENT | GD_FLG_RECORD);
	printf("%s:%d, %s(): %s: ", fname, line, func, cond);
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	putc('\n');
	uts->fail_count++;
}

ulong ut_check_free(void)
{
	struct mallinfo info = mallinfo();

	return info.uordblks;
}

long ut_check_delta(ulong last)
{
	return ut_check_free() - last;
}

static int readline_check(struct unit_test_state *uts)
{
	int ret;

	ret = console_record_readline(uts->actual_str, sizeof(uts->actual_str));
	if (ret == -ENOSPC) {
		ut_fail(uts, __FILE__, __LINE__, __func__,
			"Console record buffer too small - increase CONFIG_CONSOLE_RECORD_OUT_SIZE");
		return ret;
	}

	return 0;
}

int ut_check_console_line(struct unit_test_state *uts, const char *fmt, ...)
{
	va_list args;
	int len;
	int ret;

	va_start(args, fmt);
	len = vsnprintf(uts->expect_str, sizeof(uts->expect_str), fmt, args);
	va_end(args);
	if (len >= sizeof(uts->expect_str)) {
		ut_fail(uts, __FILE__, __LINE__, __func__,
			"unit_test_state->expect_str too small");
		return -EOVERFLOW;
	}
	ret = readline_check(uts);
	if (ret < 0)
		return ret;

	return strcmp(uts->expect_str, uts->actual_str);
}

int ut_check_console_linen(struct unit_test_state *uts, const char *fmt, ...)
{
	va_list args;
	int len;
	int ret;

	va_start(args, fmt);
	len = vsnprintf(uts->expect_str, sizeof(uts->expect_str), fmt, args);
	va_end(args);
	if (len >= sizeof(uts->expect_str)) {
		ut_fail(uts, __FILE__, __LINE__, __func__,
			"unit_test_state->expect_str too small");
		return -EOVERFLOW;
	}
	ret = readline_check(uts);
	if (ret < 0)
		return ret;

	return strncmp(uts->expect_str, uts->actual_str,
		       strlen(uts->expect_str));
}

int ut_check_skipline(struct unit_test_state *uts)
{
	int ret;

	if (!console_record_avail())
		return -ENFILE;
	ret = readline_check(uts);
	if (ret < 0)
		return ret;

	return 0;
}

int ut_check_skip_to_linen(struct unit_test_state *uts, const char *fmt, ...)
{
	va_list args;
	int len;
	int ret;

	va_start(args, fmt);
	len = vsnprintf(uts->expect_str, sizeof(uts->expect_str), fmt, args);
	va_end(args);
	if (len >= sizeof(uts->expect_str)) {
		ut_fail(uts, __FILE__, __LINE__, __func__,
			"unit_test_state->expect_str too small");
		return -EOVERFLOW;
	}
	while (1) {
		if (!console_record_avail())
			return -ENOENT;
		ret = readline_check(uts);
		if (ret < 0)
			return ret;

		if (!strncmp(uts->expect_str, uts->actual_str,
			     strlen(uts->expect_str)))
			return 0;
	}
}

int ut_check_skip_to_line(struct unit_test_state *uts, const char *fmt, ...)
{
	va_list args;
	int len;
	int ret;

	va_start(args, fmt);
	len = vsnprintf(uts->expect_str, sizeof(uts->expect_str), fmt, args);
	va_end(args);
	if (len >= sizeof(uts->expect_str)) {
		ut_fail(uts, __FILE__, __LINE__, __func__,
			"unit_test_state->expect_str too small");
		return -EOVERFLOW;
	}
	while (1) {
		if (!console_record_avail())
			return -ENOENT;
		ret = readline_check(uts);
		if (ret < 0)
			return ret;

		if (!strcmp(uts->expect_str, uts->actual_str))
			return 0;
	}
}

int ut_check_console_end(struct unit_test_state *uts)
{
	int ret;

	if (!console_record_avail())
		return 0;
	ret = readline_check(uts);
	if (ret < 0)
		return ret;

	return 1;
}

int ut_check_console_dump(struct unit_test_state *uts, int total_bytes)
{
	char *str = uts->actual_str;
	int upto;

	/* Handle empty dump */
	if (!total_bytes)
		return 0;

	for (upto = 0; upto < total_bytes;) {
		int len;
		int bytes;

		len = console_record_readline(str, sizeof(uts->actual_str));
		if (str[8] != ':' || str[9] != ' ')
			return 1;

		bytes = len - 8 - 2 - 3 * 16 - 2;
		upto += bytes;
	}

	return upto == total_bytes ? 0 : 1;
}

void ut_silence_console(struct unit_test_state *uts)
{
#ifdef CONFIG_SANDBOX
	struct sandbox_state *state = state_get_current();

	if (!state->show_test_output)
		gd->flags |= GD_FLG_SILENT;
#endif
}

void ut_unsilence_console(struct unit_test_state *uts)
{
	gd->flags &= ~(GD_FLG_SILENT | GD_FLG_RECORD);
}

void ut_set_skip_delays(struct unit_test_state *uts, bool skip_delays)
{
#ifdef CONFIG_SANDBOX
	state_set_skip_delays(skip_delays);
#endif
}
