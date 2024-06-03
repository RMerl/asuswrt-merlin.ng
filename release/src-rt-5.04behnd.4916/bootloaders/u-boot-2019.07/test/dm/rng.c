// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019, Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <rng.h>
#include <dm/test.h>
#include <test/ut.h>

/* Basic test of the rng uclass */
static int dm_test_rng_read(struct unit_test_state *uts)
{
	unsigned long rand1 = 0, rand2 = 0;
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_RNG, 0, &dev));
	ut_assertnonnull(dev);
	ut_assertok(dm_rng_read(dev, &rand1, sizeof(rand1)));
	ut_assertok(dm_rng_read(dev, &rand2, sizeof(rand2)));
	ut_assert(rand1 != rand2);

	return 0;
}
DM_TEST(dm_test_rng_read, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
