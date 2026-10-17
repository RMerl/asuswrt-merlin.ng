// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <bootcount.h>
#include <log.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_bootcount_rtc(struct unit_test_state *uts)
{
	struct udevice *dev;
	u32 val;

	ut_assertok(uclass_get_device_by_name(UCLASS_BOOTCOUNT, "bootcount@0",
					      &dev));
	ut_assertok(dm_bootcount_set(dev, 0));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0);
	ut_assertok(dm_bootcount_set(dev, 0xab));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0xab);

	ut_assertok(uclass_get_device(UCLASS_BOOTCOUNT, 1, &dev));
	ut_assertok(dm_bootcount_set(dev, 0));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0);
	ut_assertok(dm_bootcount_set(dev, 0xab));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0xab);

	return 0;
}

DM_TEST(dm_test_bootcount_rtc, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_bootcount_syscon_four_bytes(struct unit_test_state *uts)
{
	struct udevice *dev;
	u32 val;

	sandbox_set_enable_memio(true);
	ut_assertok(uclass_get_device_by_name(UCLASS_BOOTCOUNT, "bootcount_4@0",
					      &dev));
	ut_assertok(dm_bootcount_set(dev, 0xab));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0xab);
	ut_assertok(dm_bootcount_set(dev, 0));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0);

	return 0;
}

DM_TEST(dm_test_bootcount_syscon_four_bytes,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_bootcount_syscon_two_bytes(struct unit_test_state *uts)
{
	struct udevice *dev;
	u32 val;

	sandbox_set_enable_memio(true);
	ut_assertok(uclass_get_device_by_name(UCLASS_BOOTCOUNT, "bootcount_2@0",
					      &dev));
	ut_assertok(dm_bootcount_set(dev, 0xab));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0xab);
	ut_assertok(dm_bootcount_set(dev, 0));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0);

	return 0;
}

DM_TEST(dm_test_bootcount_syscon_two_bytes,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
