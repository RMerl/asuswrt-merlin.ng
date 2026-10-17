// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Google LLC
 *
 * Logging function tests for CONFIG_LOG=n without #define DEBUG
 */

#include <common.h>
#include <console.h>
#include <log.h>
#include <asm/global_data.h>
#include <test/log.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFSIZE 32

static int log_test_log_disabled_ndebug(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];
	int i;

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();

	/* Output a log record at every level */
	for (i = LOGL_EMERG; i < LOGL_COUNT; i++)
		log(LOGC_NONE, i, "testing level %i\n", i);
	gd->flags &= ~GD_FLG_RECORD;

	/* Since DEBUG is not defined, we expect to not get debug output */
	for (i = LOGL_EMERG; i < LOGL_DEBUG; i++)
		ut_assertok(ut_check_console_line(uts, "testing level %d", i));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
LOG_TEST(log_test_log_disabled_ndebug);
