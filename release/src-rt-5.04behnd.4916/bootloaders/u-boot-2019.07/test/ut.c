// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple unit test library
 *
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
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
