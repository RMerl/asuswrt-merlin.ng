// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for EFI_MEDIA uclass
 *
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that we can use the EFI_MEDIA uclass */
static int dm_test_efi_media(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_EFI_MEDIA, &dev));

	return 0;
}
DM_TEST(dm_test_efi_media, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
