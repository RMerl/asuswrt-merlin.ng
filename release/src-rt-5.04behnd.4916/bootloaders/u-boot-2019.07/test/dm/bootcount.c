// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) 2018 Theobroma Systems Design und Consulting GmbH
 */

#include <common.h>
#include <dm.h>
#include <bootcount.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_bootcount(struct unit_test_state *uts)
{
	struct udevice *dev;
	u32 val;

	ut_assertok(uclass_get_device(UCLASS_BOOTCOUNT, 0, &dev));
	ut_assertok(dm_bootcount_set(dev, 0));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0);
	ut_assertok(dm_bootcount_set(dev, 0xab));
	ut_assertok(dm_bootcount_get(dev, &val));
	ut_assert(val == 0xab);

	return 0;
}

DM_TEST(dm_test_bootcount, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

