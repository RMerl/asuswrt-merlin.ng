// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <asm/power-domain.h>
#include <test/ut.h>

/* This must match the specifier for power-domains in the DT node */
#define TEST_POWER_DOMAIN 2

static int dm_test_power_domain(struct unit_test_state *uts)
{
	struct udevice *dev_power_domain;
	struct udevice *dev_test;

	ut_assertok(uclass_get_device_by_name(UCLASS_POWER_DOMAIN,
					      "power-domain",
					      &dev_power_domain));
	ut_asserteq(0, sandbox_power_domain_query(dev_power_domain, 0));
	ut_asserteq(0, sandbox_power_domain_query(dev_power_domain,
						  TEST_POWER_DOMAIN));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "power-domain-test",
					      &dev_test));
	ut_asserteq(1, sandbox_power_domain_query(dev_power_domain,
						  TEST_POWER_DOMAIN));
	ut_assertok(sandbox_power_domain_test_get(dev_test));

	ut_assertok(sandbox_power_domain_test_on(dev_test));
	ut_asserteq(0, sandbox_power_domain_query(dev_power_domain, 0));
	ut_asserteq(1, sandbox_power_domain_query(dev_power_domain,
						  TEST_POWER_DOMAIN));

	ut_assertok(sandbox_power_domain_test_off(dev_test));
	ut_asserteq(0, sandbox_power_domain_query(dev_power_domain, 0));
	ut_asserteq(0, sandbox_power_domain_query(dev_power_domain,
						  TEST_POWER_DOMAIN));

	ut_assertok(sandbox_power_domain_test_free(dev_test));

	return 0;
}
DM_TEST(dm_test_power_domain, DM_TESTF_SCAN_FDT);
