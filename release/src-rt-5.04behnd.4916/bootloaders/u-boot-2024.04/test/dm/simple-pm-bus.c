// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <test/ut.h>
#include <asm/clk.h>
#include <asm/power-domain.h>

/* These must match the ids in the device tree */
#define TEST_CLOCK_ID 4
#define TEST_POWER_ID 1

static int dm_test_simple_pm_bus(struct unit_test_state *uts)
{
	struct udevice *power;
	struct udevice *clock;
	struct udevice *bus;

	ut_assertok(uclass_get_device_by_name(UCLASS_POWER_DOMAIN,
					      "power-domain", &power));
	ut_assertok(uclass_get_device_by_name(UCLASS_CLK, "clk-sbox",
					      &clock));
	ut_asserteq(0, sandbox_power_domain_query(power, TEST_POWER_ID));
	ut_asserteq(0, sandbox_clk_query_enable(clock, TEST_CLOCK_ID));

	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS, "pm-bus-test",
					      &bus));
	ut_asserteq(1, sandbox_power_domain_query(power, TEST_POWER_ID));
	ut_asserteq(1, sandbox_clk_query_enable(clock, TEST_CLOCK_ID));

	ut_assertok(device_remove(bus, DM_REMOVE_NORMAL));
	/* must re-probe since device_remove also removes the power domain */
	ut_assertok(uclass_get_device_by_name(UCLASS_POWER_DOMAIN,
					      "power-domain", &power));
	ut_asserteq(0, sandbox_power_domain_query(power, TEST_POWER_ID));
	ut_asserteq(0, sandbox_clk_query_enable(clock, TEST_CLOCK_ID));

	return 0;
}
DM_TEST(dm_test_simple_pm_bus, UT_TESTF_SCAN_FDT);
