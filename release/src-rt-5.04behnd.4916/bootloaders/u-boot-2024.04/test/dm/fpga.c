// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Alexander Dahl <post@lespocky.de>
 */

#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_fpga(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_FPGA, &dev));

	return 0;
}

DM_TEST(dm_test_fpga, UT_TESTF_SCAN_FDT);
