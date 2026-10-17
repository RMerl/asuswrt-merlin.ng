// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <pch.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that sandbox PCH works correctly */
static int dm_test_pch_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	u32 gbase, iobase;
	ulong sbase;

	ut_assertok(uclass_first_device_err(UCLASS_PCH, &dev));
	ut_assertok(pch_get_spi_base(dev, &sbase));
	ut_asserteq(0x10, sbase);

	ut_asserteq(0, sandbox_get_pch_spi_protect(dev));
	ut_assertok(pch_set_spi_protect(dev, true));
	ut_asserteq(1, sandbox_get_pch_spi_protect(dev));

	ut_assertok(pch_get_gpio_base(dev, &gbase));
	ut_asserteq(0x20, gbase);

	ut_assertok(pch_get_io_base(dev, &iobase));
	ut_asserteq(0x30, iobase);

	return 0;
}
DM_TEST(dm_test_pch_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test sandbox PCH ioctl */
static int dm_test_pch_ioctl(struct unit_test_state *uts)
{
	struct udevice *dev;
	char data;

	ut_assertok(uclass_first_device_err(UCLASS_PCH, &dev));

	ut_asserteq(-ENOSYS, pch_ioctl(dev, PCH_REQ_TEST1, NULL, 0));

	ut_asserteq('a', pch_ioctl(dev, PCH_REQ_TEST2, "a", 1));

	ut_asserteq(1, pch_ioctl(dev, PCH_REQ_TEST3, &data, 1));
	ut_asserteq('x', data);

	return 0;
}
DM_TEST(dm_test_pch_ioctl, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
