// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <common.h>
#include <dm.h>
#include <timer.h>
#include <dm/test.h>
#include <test/ut.h>

/*
 * Basic test of the timer uclass.
 */
static int dm_test_timer_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_TIMER, 0, &dev));
	ut_asserteq(1000000, timer_get_rate(dev));

	return 0;
}
DM_TEST(dm_test_timer_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
