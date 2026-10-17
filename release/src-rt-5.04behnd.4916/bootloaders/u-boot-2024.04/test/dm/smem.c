// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Ramon Fried <ramon.fried@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <smem.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the smem uclass */
static int dm_test_smem_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	size_t size;

	ut_assertok(uclass_get_device(UCLASS_SMEM, 0, &dev));
	ut_assertnonnull(dev);
	ut_assertok(smem_alloc(dev, -1, 0, 16));
	ut_asserteq(0, smem_get_free_space(dev, -1));
	ut_assertnull(smem_get(dev, -1, 0, &size));

	return 0;
}
DM_TEST(dm_test_smem_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
