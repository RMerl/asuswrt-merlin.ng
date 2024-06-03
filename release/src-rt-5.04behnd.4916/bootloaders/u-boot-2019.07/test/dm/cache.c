// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>

static int dm_test_reset(struct unit_test_state *uts)
{
	struct udevice *dev_cache;
	struct cache_info;

	ut_assertok(uclass_get_device(UCLASS_CACHE, 0, &dev_cache));
	ut_assertok(cache_get_info(dev, &info));

	return 0;
}
DM_TEST(dm_test_reset, DM_TESTF_SCAN_FDT);
