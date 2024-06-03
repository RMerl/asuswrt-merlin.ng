// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Simple RTC sanity check */
static int dm_test_rtc_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_RTC, 2, &dev));
	ut_assertok(uclass_get_device(UCLASS_RTC, 0, &dev));
	ut_assertok(uclass_get_device(UCLASS_RTC, 1, &dev));

	return 0;
}
DM_TEST(dm_test_rtc_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static void show_time(const char *msg, struct rtc_time *time)
{
	printf("%s: %02d/%02d/%04d %02d:%02d:%02d\n", msg,
	       time->tm_mday, time->tm_mon, time->tm_year,
	       time->tm_hour, time->tm_min, time->tm_sec);
}

static int cmp_times(struct rtc_time *expect, struct rtc_time *time, bool show)
{
	bool same;

	same = expect->tm_sec == time->tm_sec;
	same &= expect->tm_min == time->tm_min;
	same &= expect->tm_hour == time->tm_hour;
	same &= expect->tm_mday == time->tm_mday;
	same &= expect->tm_mon == time->tm_mon;
	same &= expect->tm_year == time->tm_year;
	if (!same && show) {
		show_time("expected", expect);
		show_time("actual", time);
	}

	return same ? 0 : -EINVAL;
}

/* Set and get the time */
static int dm_test_rtc_set_get(struct unit_test_state *uts)
{
	struct rtc_time now, time, cmp;
	struct udevice *dev, *emul;
	long offset, old_offset, old_base_time;

	ut_assertok(uclass_get_device(UCLASS_RTC, 0, &dev));
	ut_assertok(dm_rtc_get(dev, &now));

	ut_assertok(i2c_emul_find(dev, &emul));
	ut_assert(emul != NULL);

	/* Tell the RTC to go into manual mode */
	old_offset = sandbox_i2c_rtc_set_offset(emul, false, 0);
	old_base_time = sandbox_i2c_rtc_get_set_base_time(emul, -1);

	memset(&time, '\0', sizeof(time));
	time.tm_mday = 25;
	time.tm_mon = 8;
	time.tm_year = 2004;
	time.tm_sec = 0;
	time.tm_min = 18;
	time.tm_hour = 18;
	ut_assertok(dm_rtc_set(dev, &time));

	memset(&cmp, '\0', sizeof(cmp));
	ut_assertok(dm_rtc_get(dev, &cmp));
	ut_assertok(cmp_times(&time, &cmp, true));

	/* Increment by 1 second */
	offset = sandbox_i2c_rtc_set_offset(emul, false, 0);
	sandbox_i2c_rtc_set_offset(emul, false, offset + 1);

	memset(&cmp, '\0', sizeof(cmp));
	ut_assertok(dm_rtc_get(dev, &cmp));
	ut_asserteq(1, cmp.tm_sec);

	/* Check against original offset */
	sandbox_i2c_rtc_set_offset(emul, false, old_offset);
	ut_assertok(dm_rtc_get(dev, &cmp));
	ut_assertok(cmp_times(&now, &cmp, true));

	/* Back to the original offset */
	sandbox_i2c_rtc_set_offset(emul, false, 0);
	memset(&cmp, '\0', sizeof(cmp));
	ut_assertok(dm_rtc_get(dev, &cmp));
	ut_assertok(cmp_times(&now, &cmp, true));

	/* Increment the base time by 1 emul */
	sandbox_i2c_rtc_get_set_base_time(emul, old_base_time + 1);
	memset(&cmp, '\0', sizeof(cmp));
	ut_assertok(dm_rtc_get(dev, &cmp));
	if (now.tm_sec == 59) {
		ut_asserteq(0, cmp.tm_sec);
	} else {
		ut_asserteq(now.tm_sec + 1, cmp.tm_sec);
	}

	old_offset = sandbox_i2c_rtc_set_offset(emul, true, 0);

	return 0;
}
DM_TEST(dm_test_rtc_set_get, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Reset the time */
static int dm_test_rtc_reset(struct unit_test_state *uts)
{
	struct rtc_time now;
	struct udevice *dev, *emul;
	long old_base_time, base_time;

	ut_assertok(uclass_get_device(UCLASS_RTC, 0, &dev));
	ut_assertok(dm_rtc_get(dev, &now));

	ut_assertok(i2c_emul_find(dev, &emul));
	ut_assert(emul != NULL);

	old_base_time = sandbox_i2c_rtc_get_set_base_time(emul, 0);

	ut_asserteq(0, sandbox_i2c_rtc_get_set_base_time(emul, -1));

	/* Resetting the RTC should put he base time back to normal */
	ut_assertok(dm_rtc_reset(dev));
	base_time = sandbox_i2c_rtc_get_set_base_time(emul, -1);
	ut_asserteq(old_base_time, base_time);

	return 0;
}
DM_TEST(dm_test_rtc_reset, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Check that two RTC devices can be used independently */
static int dm_test_rtc_dual(struct unit_test_state *uts)
{
	struct rtc_time now1, now2, cmp;
	struct udevice *dev1, *dev2;
	struct udevice *emul1, *emul2;
	long offset;

	ut_assertok(uclass_get_device(UCLASS_RTC, 0, &dev1));
	ut_assertok(dm_rtc_get(dev1, &now1));
	ut_assertok(uclass_get_device(UCLASS_RTC, 1, &dev2));
	ut_assertok(dm_rtc_get(dev2, &now2));

	ut_assertok(i2c_emul_find(dev1, &emul1));
	ut_assert(emul1 != NULL);
	ut_assertok(i2c_emul_find(dev2, &emul2));
	ut_assert(emul2 != NULL);

	offset = sandbox_i2c_rtc_set_offset(emul1, false, -1);
	sandbox_i2c_rtc_set_offset(emul2, false, offset + 1);
	memset(&cmp, '\0', sizeof(cmp));
	ut_assertok(dm_rtc_get(dev2, &cmp));
	ut_asserteq(-EINVAL, cmp_times(&now1, &cmp, false));

	memset(&cmp, '\0', sizeof(cmp));
	ut_assertok(dm_rtc_get(dev1, &cmp));
	ut_assertok(cmp_times(&now1, &cmp, true));

	return 0;
}
DM_TEST(dm_test_rtc_dual, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
