// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <console.h>
#include <log.h>
#include <asm/global_data.h>
#include <test/log.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test invalid options */
static int log_test_filter_invalid(struct unit_test_state *uts)
{
	ut_asserteq(1, run_command("log filter-add -AD", 0));
	ut_asserteq(1, run_command("log filter-add -l1 -L1", 0));
	ut_asserteq(1, run_command("log filter-add -l1 -L1", 0));
	ut_asserteq(1, run_command("log filter-add -lfoo", 0));
	ut_asserteq(1, run_command("log filter-add -cfoo", 0));
	ut_asserteq(1, run_command("log filter-add -ccore -ccore -ccore -ccore "
				   "-ccore -ccore", 0));

	return 0;
}
LOG_TEST_FLAGS(log_test_filter_invalid, UT_TESTF_CONSOLE_REC);

/* Test adding and removing filters */
static int log_test_filter(struct unit_test_state *uts)
{
	bool any_found = false;
	bool filt1_found = false;
	bool filt2_found = false;
	char cmd[32];
	struct log_filter *filt;
	struct log_device *ldev;
	ulong filt1, filt2;

#define create_filter(args, filter_num) do {\
	ut_assertok(console_record_reset_enable()); \
	ut_assertok(run_command("log filter-add -p " args, 0)); \
	ut_assert_skipline(); \
	ut_assertok(strict_strtoul(uts->actual_str, 10, &(filter_num))); \
	ut_assert_console_end(); \
} while (0)

	create_filter("", filt1);
	create_filter("-DL warning -cmmc -cspi -ffile", filt2);

	ldev = log_device_find_by_name("console");
	ut_assertnonnull(ldev);
	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		if (filt->filter_num == filt1) {
			filt1_found = true;
			ut_asserteq(0, filt->flags);
			ut_asserteq(LOGL_MAX, filt->level);
			ut_assertnull(filt->file_list);
		} else if (filt->filter_num == filt2) {
			filt2_found = true;
			ut_asserteq(LOGFF_HAS_CAT | LOGFF_DENY |
				    LOGFF_LEVEL_MIN, filt->flags);
			ut_asserteq(true, log_has_cat(filt->cat_list,
						      log_uc_cat(UCLASS_MMC)));
			ut_asserteq(true, log_has_cat(filt->cat_list,
						      log_uc_cat(UCLASS_SPI)));
			ut_asserteq(LOGL_WARNING, filt->level);
			ut_asserteq_str("file", filt->file_list);
		}
	}
	ut_asserteq(true, filt1_found);
	ut_asserteq(true, filt2_found);

#define remove_filter(filter_num) do { \
	ut_assertok(console_record_reset_enable()); \
	snprintf(cmd, sizeof(cmd), "log filter-remove %lu", filter_num); \
	ut_assertok(run_command(cmd, 0)); \
	ut_assert_console_end(); \
} while (0)

	remove_filter(filt1);
	remove_filter(filt2);

	filt1_found = false;
	filt2_found = false;
	list_for_each_entry(filt, &ldev->filter_head, sibling_node) {
		if (filt->filter_num == filt1)
			filt1_found = true;
		else if (filt->filter_num == filt2)
			filt2_found = true;
	}
	ut_asserteq(false, filt1_found);
	ut_asserteq(false, filt2_found);

	create_filter("", filt1);
	create_filter("", filt2);

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("log filter-remove -a", 0));
	ut_assert_console_end();

	list_for_each_entry(filt, &ldev->filter_head, sibling_node)
		any_found = true;
	ut_asserteq(false, any_found);

	return 0;
}
LOG_TEST_FLAGS(log_test_filter, UT_TESTF_CONSOLE_REC);
