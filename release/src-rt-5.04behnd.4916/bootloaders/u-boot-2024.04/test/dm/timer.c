// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <common.h>
#include <dm.h>
#include <timer.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <test/test.h>
#include <test/ut.h>
#include <asm/cpu.h>

/*
 * Basic test of the timer uclass.
 */
static int dm_test_timer_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_TIMER, "timer@0", &dev));
	ut_asserteq(1000000, timer_get_rate(dev));

	return 0;
}
DM_TEST(dm_test_timer_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/*
 * Test of timebase fallback
 */
static int dm_test_timer_timebase_fallback(struct unit_test_state *uts)
{
	struct udevice *dev;

	cpu_sandbox_set_current("cpu@1");
	ut_assertok(uclass_get_device_by_name(UCLASS_TIMER, "timer@1", &dev));
	ut_asserteq(3000000, timer_get_rate(dev));
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));

	cpu_sandbox_set_current("cpu@2");
	ut_assertok(uclass_get_device_by_name(UCLASS_TIMER, "timer@1", &dev));
	ut_asserteq(2000000, timer_get_rate(dev));

	cpu_sandbox_set_current("cpu@1");

	return 0;
}
DM_TEST(dm_test_timer_timebase_fallback,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
