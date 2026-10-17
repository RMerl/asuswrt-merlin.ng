// SPDX-License-Identifier: GPL-2.0+
/*
 * Logging support test program
 *
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <asm/global_data.h>
#include <test/log.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* emit some sample log records in different ways, for testing */
static int do_log_run(struct unit_test_state *uts, int cat, const char *file)
{
	int i;
	int ret, expected_ret;

	if (gd->flags & GD_FLG_LOG_READY)
		expected_ret = 0;
	else
		expected_ret = -ENOSYS;

	gd->log_fmt = LOGF_TEST;
	debug("debug\n");
	for (i = LOGL_FIRST; i < LOGL_COUNT; i++) {
		log(cat, i, "log %d\n", i);
		ret = _log(log_uc_cat(cat), i, file, 100 + i,
			   "func", "_log %d\n", i);
		ut_asserteq(ret, expected_ret);
	}
	/* test with LOGL_COUNT flag */
	for (i = LOGL_FIRST; i < LOGL_COUNT; i++) {
		ret = _log(log_uc_cat(cat), i | LOGL_FORCE_DEBUG, file, 100 + i,
			   "func", "_log force %d\n", i);
		ut_asserteq(ret, expected_ret);
	}

	gd->log_fmt = log_get_default_format();
	return 0;
}

#define log_run_cat(cat) do_log_run(uts, cat, "file")
#define log_run_file(file) do_log_run(uts, UCLASS_SPI, file)
#define log_run() do_log_run(uts, UCLASS_SPI, "file")

#define EXPECT_LOG BIT(0)
#define EXPECT_DIRECT BIT(1)
#define EXPECT_EXTRA BIT(2)
#define EXPECT_FORCE BIT(3)
#define EXPECT_DEBUG BIT(4)

static int do_check_log_entries(struct unit_test_state *uts, int flags, int min,
				int max)
{
	int i;

	for (i = min; i <= max; i++) {
		if (flags & EXPECT_LOG)
			ut_assert_nextline("          do_log_run() log %d", i);
		if (flags & EXPECT_DIRECT)
			ut_assert_nextline("                func() _log %d", i);
		if (flags & EXPECT_DEBUG) {
			ut_assert_nextline("log %d", i);
			ut_assert_nextline("_log %d", i);
		}
	}
	if (flags & EXPECT_EXTRA)
		for (; i <= LOGL_MAX ; i++)
			ut_assert_nextline("                func() _log %d", i);

	for (i = LOGL_FIRST; i < LOGL_COUNT; i++) {
		if (flags & EXPECT_FORCE)
			ut_assert_nextline("                func() _log force %d",
					   i);
		if (flags & EXPECT_DEBUG)
			ut_assert_nextline("_log force %d", i);
	}

	ut_assert_console_end();
	return 0;
}

#define check_log_entries_flags_levels(flags, min, max) do {\
	int ret = do_check_log_entries(uts, flags, min, max); \
	if (ret) \
		return ret; \
} while (0)

#define check_log_entries_flags(flags) \
	check_log_entries_flags_levels(flags, LOGL_FIRST, _LOG_MAX_LEVEL)
#define check_log_entries() check_log_entries_flags(EXPECT_LOG | EXPECT_DIRECT | EXPECT_FORCE)
#define check_log_entries_extra() \
	check_log_entries_flags(EXPECT_LOG | EXPECT_DIRECT | EXPECT_EXTRA | EXPECT_FORCE)
#define check_log_entries_none() check_log_entries_flags(EXPECT_FORCE)

/* Check a category filter using the first category */
int log_test_cat_allow(struct unit_test_state *uts)
{
	enum log_category_t cat_list[] = {
		log_uc_cat(UCLASS_MMC), log_uc_cat(UCLASS_SPI),
		LOGC_NONE, LOGC_END
	};
	int filt;

	filt = log_add_filter("console", cat_list, LOGL_MAX, NULL);
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_MMC);
	check_log_entries_extra();

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_SPI);
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_cat_allow, UT_TESTF_CONSOLE_REC);

/* Check a category filter that should block log entries */
int log_test_cat_deny_implicit(struct unit_test_state *uts)
{
	enum log_category_t cat_list[] = {
		log_uc_cat(UCLASS_MMC),  LOGC_NONE, LOGC_END
	};
	int filt;

	filt = log_add_filter("console", cat_list, LOGL_MAX, NULL);
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_SPI);
	check_log_entries_none();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_cat_deny_implicit, UT_TESTF_CONSOLE_REC);

/* Check passing and failing file filters */
int log_test_file(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_MAX, "file");
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file");
	check_log_entries_flags(EXPECT_DIRECT | EXPECT_EXTRA | EXPECT_FORCE);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_none();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_file, UT_TESTF_CONSOLE_REC);

/* Check a passing file filter (second in list) */
int log_test_file_second(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_MAX, "file,file2");
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_flags(EXPECT_DIRECT | EXPECT_EXTRA | EXPECT_FORCE);

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_file_second, UT_TESTF_CONSOLE_REC);

/* Check a passing file filter (middle of list) */
int log_test_file_mid(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_MAX,
			      "file,file2,log/log_test.c");
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_file_mid, UT_TESTF_CONSOLE_REC);

/* Check a log level filter */
int log_test_level(struct unit_test_state *uts)
{
	int filt;

	filt = log_add_filter("console", NULL, LOGL_WARNING, NULL);
	ut_assert(filt >= 0);

	ut_assertok(console_record_reset_enable());
	log_run();
	check_log_entries_flags_levels(EXPECT_LOG | EXPECT_DIRECT | EXPECT_FORCE,
				       LOGL_FIRST, LOGL_WARNING);

	ut_assertok(log_remove_filter("console", filt));
	return 0;
}
LOG_TEST_FLAGS(log_test_level, UT_TESTF_CONSOLE_REC);

/* Check two filters, one of which passes everything */
int log_test_double(struct unit_test_state *uts)
{
	int filt1, filt2;

	filt1 = log_add_filter("console", NULL, LOGL_WARNING, NULL);
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter("console", NULL, LOGL_MAX, NULL);
	ut_assert(filt2 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run();
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	return 0;
}
LOG_TEST_FLAGS(log_test_double, UT_TESTF_CONSOLE_REC);

/* Check three filters, which together pass everything */
int log_test_triple(struct unit_test_state *uts)
{
	int filt1, filt2, filt3;

	filt1 = log_add_filter("console", NULL, LOGL_MAX, "file)");
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter("console", NULL, LOGL_MAX, "file2");
	ut_assert(filt2 >= 0);
	filt3 = log_add_filter("console", NULL, LOGL_MAX, "log/log_test.c");
	ut_assert(filt3 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file2");
	check_log_entries_extra();

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	ut_assertok(log_remove_filter("console", filt3));
	return 0;
}
LOG_TEST_FLAGS(log_test_triple, UT_TESTF_CONSOLE_REC);

int do_log_test_helpers(struct unit_test_state *uts)
{
	int i;

	ut_assertok(console_record_reset_enable());
	log_err("level %d\n", LOGL_EMERG);
	log_err("level %d\n", LOGL_ALERT);
	log_err("level %d\n", LOGL_CRIT);
	log_err("level %d\n", LOGL_ERR);
	log_warning("level %d\n", LOGL_WARNING);
	log_notice("level %d\n", LOGL_NOTICE);
	log_info("level %d\n", LOGL_INFO);
	log_debug("level %d\n", LOGL_DEBUG);
	log_content("level %d\n", LOGL_DEBUG_CONTENT);
	log_io("level %d\n", LOGL_DEBUG_IO);

	for (i = LOGL_EMERG; i <= gd->default_log_level; i++)
		ut_assert_nextline("%*s() level %d", CONFIG_LOGF_FUNC_PAD,
				   __func__, i);
	ut_assert_console_end();
	return 0;
}

int log_test_helpers(struct unit_test_state *uts)
{
	int ret;

	gd->log_fmt = LOGF_TEST;
	ret = do_log_test_helpers(uts);
	gd->log_fmt = log_get_default_format();
	return ret;
}
LOG_TEST_FLAGS(log_test_helpers, UT_TESTF_CONSOLE_REC);

int do_log_test_disable(struct unit_test_state *uts)
{
	ut_assertok(console_record_reset_enable());
	log_err("default\n");
	ut_assert_nextline("%*s() default", CONFIG_LOGF_FUNC_PAD, __func__);

	ut_assertok(log_device_set_enable(LOG_GET_DRIVER(console), false));
	log_err("disabled\n");

	ut_assertok(log_device_set_enable(LOG_GET_DRIVER(console), true));
	log_err("enabled\n");
	ut_assert_nextline("%*s() enabled", CONFIG_LOGF_FUNC_PAD, __func__);
	ut_assert_console_end();
	return 0;
}

int log_test_disable(struct unit_test_state *uts)
{
	int ret;

	gd->log_fmt = LOGF_TEST;
	ret = do_log_test_disable(uts);
	gd->log_fmt = log_get_default_format();
	return ret;
}
LOG_TEST_FLAGS(log_test_disable, UT_TESTF_CONSOLE_REC);

/* Check denying based on category */
int log_test_cat_deny(struct unit_test_state *uts)
{
	int filt1, filt2;
	enum log_category_t cat_list[] = {
		log_uc_cat(UCLASS_SPI), LOGC_END
	};

	filt1 = log_add_filter("console", cat_list, LOGL_MAX, NULL);
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter_flags("console", cat_list, LOGL_MAX, NULL,
				     LOGFF_DENY);
	ut_assert(filt2 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_cat(UCLASS_SPI);
	check_log_entries_none();

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	return 0;
}
LOG_TEST_FLAGS(log_test_cat_deny, UT_TESTF_CONSOLE_REC);

/* Check denying based on file */
int log_test_file_deny(struct unit_test_state *uts)
{
	int filt1, filt2;

	filt1 = log_add_filter("console", NULL, LOGL_MAX, "file");
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter_flags("console", NULL, LOGL_MAX, "file",
				     LOGFF_DENY);
	ut_assert(filt2 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run_file("file");
	check_log_entries_none();

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	return 0;
}
LOG_TEST_FLAGS(log_test_file_deny, UT_TESTF_CONSOLE_REC);

/* Check denying based on level */
int log_test_level_deny(struct unit_test_state *uts)
{
	int filt1, filt2;

	filt1 = log_add_filter("console", NULL, LOGL_INFO, NULL);
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter_flags("console", NULL, LOGL_WARNING, NULL,
				     LOGFF_DENY);
	ut_assert(filt2 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run();
	check_log_entries_flags_levels(EXPECT_LOG | EXPECT_DIRECT | EXPECT_FORCE,
				       LOGL_WARNING + 1,
				       min(gd->default_log_level, LOGL_INFO));

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	return 0;
}
LOG_TEST_FLAGS(log_test_level_deny, UT_TESTF_CONSOLE_REC);

/* Check matching based on minimum level */
int log_test_min(struct unit_test_state *uts)
{
	int filt1, filt2;

	filt1 = log_add_filter_flags("console", NULL, LOGL_WARNING, NULL,
				     LOGFF_LEVEL_MIN);
	ut_assert(filt1 >= 0);
	filt2 = log_add_filter_flags("console", NULL, LOGL_INFO, NULL,
				     LOGFF_DENY | LOGFF_LEVEL_MIN);
	ut_assert(filt2 >= 0);

	ut_assertok(console_record_reset_enable());
	log_run();
	check_log_entries_flags_levels(EXPECT_LOG | EXPECT_DIRECT | EXPECT_FORCE,
				       LOGL_WARNING, LOGL_INFO - 1);

	ut_assertok(log_remove_filter("console", filt1));
	ut_assertok(log_remove_filter("console", filt2));
	return 0;
}
LOG_TEST_FLAGS(log_test_min, UT_TESTF_CONSOLE_REC);

/* Check dropped traces */
int log_test_dropped(struct unit_test_state *uts)
{
	/* force LOG not ready */
	gd->flags &= ~(GD_FLG_LOG_READY);
	gd->log_drop_count = 0;

	ut_assertok(console_record_reset_enable());

	log_run();
	ut_asserteq(2 * (LOGL_COUNT - LOGL_FIRST) +
		    _LOG_MAX_LEVEL - LOGL_FIRST + 1,
		    gd->log_drop_count);
	check_log_entries_flags_levels(EXPECT_DEBUG, LOGL_FIRST, CONFIG_LOG_DEFAULT_LEVEL);

	gd->flags |= GD_FLG_LOG_READY;
	gd->log_drop_count = 0;

	return 0;
}
LOG_TEST_FLAGS(log_test_dropped, UT_TESTF_CONSOLE_REC);

/* Check log_buffer() */
int log_test_buffer(struct unit_test_state *uts)
{
	u8 *buf;
	int i;

	buf = malloc(0x20);
	ut_assertnonnull(buf);
	memset(buf, '\0', 0x20);
	for (i = 0; i < 0x11; i++)
		buf[i] = i * 0x11;

	ut_assertok(console_record_reset_enable());
	log_buffer(LOGC_BOOT, LOGL_INFO, 0, buf, 1, 0x12, 0);

	/* This one should product no output due to the debug level */
	log_buffer(LOGC_BOOT, LOGL_DEBUG, 0, buf, 1, 0x12, 0);

	ut_assert_nextline("00000000: 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff  ..\"3DUfw........");
	ut_assert_nextline("00000010: 10 00                                            ..");
	ut_assert_console_end();
	free(buf);

	return 0;
}
LOG_TEST_FLAGS(log_test_buffer, UT_TESTF_CONSOLE_REC);
