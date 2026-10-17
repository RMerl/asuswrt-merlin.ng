// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test continuation of log messages using pr_cont().
 */

#include <common.h>
#include <console.h>
#include <test/log.h>
#include <test/test.h>
#include <test/suites.h>
#include <test/ut.h>
#include <asm/global_data.h>
#include <linux/printk.h>

#define BUFFSIZE 64

DECLARE_GLOBAL_DATA_PTR;

static int log_test_pr_cont(struct unit_test_state *uts)
{
	int log_fmt;
	int log_level;

	log_fmt = gd->log_fmt;
	log_level = gd->default_log_level;

	/* Write two messages, the second continuing the first */
	gd->log_fmt = BIT(LOGF_MSG);
	gd->default_log_level = LOGL_INFO;
	console_record_reset_enable();
	pr_err("ea%d ", 1);
	pr_cont("cc%d\n", 2);
	gd->default_log_level = log_level;
	gd->log_fmt = log_fmt;
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "ea1 cc2"));
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
LOG_TEST(log_test_pr_cont);
