// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <dm/test.h>
#include <extcon.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_extcon(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_EXTCON, "extcon", &dev));

	return 0;
}

DM_TEST(dm_test_extcon, UT_TESTF_SCAN_FDT);
