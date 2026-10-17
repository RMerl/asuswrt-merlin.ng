// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_memory(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_MEMORY, &dev));

	return 0;
}

DM_TEST(dm_test_memory, UT_TESTF_SCAN_FDT);
