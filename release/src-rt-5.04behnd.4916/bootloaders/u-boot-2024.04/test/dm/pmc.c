// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for power-management controller uclass (PMC)
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <power/acpi_pmc.h>
#include <dm/test.h>
#include <test/ut.h>

/* Base test of the PMC uclass */
static int dm_test_pmc_base(struct unit_test_state *uts)
{
	struct acpi_pmc_upriv *upriv;
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_ACPI_PMC, &dev));

	ut_assertok(pmc_disable_tco(dev));
	ut_assertok(pmc_init(dev));
	ut_assertok(pmc_prev_sleep_state(dev));

	/* Check some values to see that I/O works */
	upriv = dev_get_uclass_priv(dev);
	ut_asserteq(0x24, upriv->gpe0_sts[1]);
	ut_asserteq(0x64, upriv->tco1_sts);

	return 0;
}
DM_TEST(dm_test_pmc_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
