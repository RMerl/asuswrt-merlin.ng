// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Base test of firmware probe */
static int dm_test_firmware_probe(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device_by_name(UCLASS_FIRMWARE,
					      "sandbox-firmware", &dev));
	return 0;
}
DM_TEST(dm_test_firmware_probe, DM_TESTF_SCAN_FDT);
