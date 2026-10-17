// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dm/simple_bus.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

static int dm_test_simple_bus(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct simple_bus_plat *plat;

	/* locate the dummy device @ translation-test node */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));
	ut_asserteq_str("dev@0,0", dev->name);

	/* locate the parent node which is a simple-bus */
	ut_assertnonnull(dev = dev_get_parent(dev));
	ut_asserteq_str("translation-test@8000", dev->name);

	ut_assertnonnull(plat = dev_get_uclass_plat(dev));
	ut_asserteq(0, plat->base);
	ut_asserteq(0x8000, plat->target);
	ut_asserteq(0x1000, plat->size);

	return 0;
}
DM_TEST(dm_test_simple_bus, UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);
