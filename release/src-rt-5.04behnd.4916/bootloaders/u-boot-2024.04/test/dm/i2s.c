// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <i2s.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the i2s codec uclass */
static int dm_test_i2s(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 data[3];

	/* check probe success */
	ut_assertok(uclass_first_device_err(UCLASS_I2S, &dev));
	data[0] = 1;
	data[1] = 4;
	data[2] = 6;
	ut_assertok(i2s_tx_data(dev, data, ARRAY_SIZE(data)));
	ut_asserteq(11, sandbox_get_i2s_sum(dev));
	ut_assertok(i2s_tx_data(dev, data, 1));
	ut_asserteq(12, sandbox_get_i2s_sum(dev));

	return 0;
}
DM_TEST(dm_test_i2s, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
