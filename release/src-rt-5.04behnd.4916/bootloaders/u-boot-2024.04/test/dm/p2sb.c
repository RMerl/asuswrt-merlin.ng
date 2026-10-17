// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for Primary-to-Sideband bus (P2SB)
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <p2sb.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Base test of the PMC uclass */
static int dm_test_p2sb_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	sandbox_set_enable_memio(true);
	ut_assertok(uclass_get_device_by_name(UCLASS_AXI, "adder", &dev));
	ut_asserteq(0x03000004, pcr_read32(dev, 4));
	ut_asserteq(0x300, pcr_read16(dev, 6));
	ut_asserteq(4, pcr_read8(dev, 4));

	return 0;
}
DM_TEST(dm_test_p2sb_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
