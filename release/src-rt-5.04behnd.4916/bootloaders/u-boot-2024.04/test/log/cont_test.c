// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test continuation of log messages.
 */

#include <common.h>
#include <console.h>
#include <asm/global_data.h>
#include <test/log.h>
#include <test/test.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

static int log_test_cont(struct unit_test_state *uts)
{
	int log_fmt;
	int log_level;

	log_fmt = gd->log_fmt;
	log_level = gd->default_log_level;

	/* Write two messages, the second continuing the first */
	gd->log_fmt = (1 << LOGF_CAT) | (1 << LOGF_LEVEL) | (1 << LOGF_MSG);
	gd->default_log_level = LOGL_INFO;
	console_record_reset_enable();
	log(LOGC_ARCH, LOGL_ERR, "ea%d\n", 1);
	log(LOGC_CONT, LOGL_CONT, "cc%d\n", 2);
	gd->default_log_level = log_level;
	gd->log_fmt = log_fmt;
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "ERR.arch, ea1"));
	ut_assertok(ut_check_console_line(uts, "ERR.arch, cc2"));
	ut_assertok(ut_check_console_end(uts));

	/* Write a third message which is not a continuation */
	gd->log_fmt = (1 << LOGF_CAT) | (1 << LOGF_LEVEL) | (1 << LOGF_MSG);
	gd->default_log_level = LOGL_INFO;
	console_record_reset_enable();
	log(LOGC_EFI, LOGL_INFO, "ie%d\n", 3);
	gd->default_log_level = log_level;
	gd->log_fmt = log_fmt;
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "INFO.efi, ie3"));
	ut_assertok(ut_check_console_end(uts));

	/* Write two messages without a newline between them */
	gd->log_fmt = (1 << LOGF_CAT) | (1 << LOGF_LEVEL) | (1 << LOGF_MSG);
	gd->default_log_level = LOGL_INFO;
	console_record_reset_enable();
	log(LOGC_ARCH, LOGL_ERR, "ea%d ", 1);
	log(LOGC_CONT, LOGL_CONT, "cc%d\n", 2);
	gd->default_log_level = log_level;
	gd->log_fmt = log_fmt;
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "ERR.arch, ea1 cc2"));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
LOG_TEST(log_test_cont);
