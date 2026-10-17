// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Logging function tests for CONFIG_LOG=n.
 */

/* Needed for testing log_debug() */
#define DEBUG 1

#include <common.h>
#include <console.h>
#include <log.h>
#include <asm/global_data.h>
#include <test/log.h>
#include <test/test.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

#define BUFFSIZE 32

static int log_test_nolog_err(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	log_err("testing %s\n", "log_err");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "testing log_err"));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(log_test_nolog_err);

static int log_test_nolog_warning(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	log_warning("testing %s\n", "log_warning");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "testing log_warning"));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(log_test_nolog_warning);

static int log_test_nolog_notice(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	log_notice("testing %s\n", "log_notice");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "testing log_notice"));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(log_test_nolog_notice);

static int log_test_nolog_info(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	log_err("testing %s\n", "log_info");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "testing log_info"));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(log_test_nolog_info);

#undef _DEBUG
#define _DEBUG 0
static int nolog_test_nodebug(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	debug("testing %s\n", "debug");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(nolog_test_nodebug);

static int log_test_nolog_nodebug(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	log_debug("testing %s\n", "log_debug");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assert(!strcmp(buf, ""));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(log_test_nolog_nodebug);

#undef _DEBUG
#define _DEBUG 1
static int nolog_test_debug(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	debug("testing %s\n", "debug");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "testing debug"));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(nolog_test_debug);

static int log_test_nolog_debug(struct unit_test_state *uts)
{
	char buf[BUFFSIZE];

	memset(buf, 0, BUFFSIZE);
	console_record_reset_enable();
	log_debug("testing %s\n", "log_debug");
	log(LOGC_NONE, LOGL_DEBUG, "more %s\n", "log_debug");
	gd->flags &= ~GD_FLG_RECORD;
	ut_assertok(ut_check_console_line(uts, "testing log_debug"));
	ut_assertok(ut_check_console_line(uts, "more log_debug"));
	ut_assertok(ut_check_console_end(uts));
	return 0;
}
LOG_TEST(log_test_nolog_debug);
