// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <ram.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Basic test of the ram uclass */
static int dm_test_ram_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct ram_info info;

	ut_assertok(uclass_get_device(UCLASS_RAM, 0, &dev));
	ut_assertok(ram_get_info(dev, &info));
	ut_asserteq(0, info.base);
	ut_asserteq(gd->ram_size, info.size);

	return 0;
}
DM_TEST(dm_test_ram_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
