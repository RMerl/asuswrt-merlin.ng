// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - https://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Base test of the phy uclass */
static int dm_test_phy_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct phy phy1_method1;
	struct phy phy1_method2;
	struct phy phy2;
	struct phy phy3;
	struct udevice *parent;

	/* Get the device using the phy device*/
	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user", &parent));
	/*
	 * Get the same phy port in 2 different ways and compare.
	 */
	ut_assertok(generic_phy_get_by_name(parent, "phy1", &phy1_method1));
	ut_assert(generic_phy_valid(&phy1_method1));
	ut_assertok(generic_phy_get_by_index(parent, 0, &phy1_method2));
	ut_assert(generic_phy_valid(&phy1_method2));
	ut_asserteq(phy1_method1.id, phy1_method2.id);

	/*
	 * Get the second phy port. Check that the same phy provider (device)
	 * provides this 2nd phy port, but that the IDs are different
	 */
	ut_assertok(generic_phy_get_by_name(parent, "phy2", &phy2));
	ut_asserteq_ptr(phy1_method2.dev, phy2.dev);
	ut_assert(phy1_method1.id != phy2.id);

	/*
	 * Get the third phy port. Check that the phy provider is different
	 */
	ut_assertok(generic_phy_get_by_name(parent, "phy3", &phy3));
	ut_assert(phy2.dev != phy3.dev);

	/* Try to get a non-existing phy */
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_PHY, 5, &dev));
	ut_asserteq(-ENODATA, generic_phy_get_by_name(parent,
					"phy_not_existing", &phy1_method1));
	ut_assert(!generic_phy_valid(&phy1_method1));
	ut_asserteq(-ENOENT, generic_phy_get_by_index(parent, 3,
						      &phy1_method2));
	ut_assert(!generic_phy_valid(&phy1_method2));

	/* Try to get a phy where of_xlate fail */
	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user2", &parent));
	ut_asserteq(-EINVAL, generic_phy_get_by_name(parent, "phy1",
						     &phy1_method1));
	ut_assert(!generic_phy_valid(&phy1_method1));
	ut_asserteq(-EINVAL, generic_phy_get_by_index(parent, 0,
						      &phy1_method2));
	ut_assert(!generic_phy_valid(&phy1_method2));

	return 0;
}
DM_TEST(dm_test_phy_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test of the phy uclass using the sandbox phy driver operations */
static int dm_test_phy_ops(struct unit_test_state *uts)
{
	struct phy phy1;
	struct phy phy2;
	struct phy phy3;
	struct udevice *parent;

	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user", &parent));

	ut_assertok(generic_phy_get_by_name(parent, "phy1", &phy1));
	ut_asserteq(0, phy1.id);
	ut_assertok(generic_phy_get_by_name(parent, "phy2", &phy2));
	ut_asserteq(1, phy2.id);
	ut_assertok(generic_phy_get_by_name(parent, "phy3", &phy3));
	ut_asserteq(0, phy3.id);

	/* test normal operations */
	ut_assertok(generic_phy_init(&phy1));
	ut_assertok(generic_phy_power_on(&phy1));
	ut_assertok(generic_phy_power_off(&phy1));

	/*
	 * Test power_on() failure after exit().
	 * The sandbox phy driver does not allow power-on/off after
	 * exit, but the uclass counts power-on/init calls and skips
	 * calling the driver's ops when e.g. powering off an already
	 * powered-off phy.
	 */
	ut_assertok(generic_phy_exit(&phy1));
	ut_assert(generic_phy_power_on(&phy1) != 0);
	ut_assertok(generic_phy_power_off(&phy1));

	/*
	 * test normal operations again (after re-init)
	 */
	ut_assertok(generic_phy_init(&phy1));
	ut_assertok(generic_phy_power_on(&phy1));
	ut_assertok(generic_phy_power_off(&phy1));

	/*
	 * test calling unimplemented feature.
	 * The call is expected to succeed
	 */
	ut_assertok(generic_phy_reset(&phy1));

	/*
	 * Test power_off() failure after exit().
	 * For this we need to call exit() while the phy is powered-on,
	 * so that the uclass actually calls the driver's power-off()
	 * and reports the resulting failure.
	 */
	ut_assertok(generic_phy_power_on(&phy1));
	ut_assertok(generic_phy_exit(&phy1));
	ut_assert(generic_phy_power_off(&phy1) != 0);
	ut_assertok(generic_phy_power_on(&phy1));

	/* PHY2 has a known problem with power off */
	ut_assertok(generic_phy_init(&phy2));
	ut_assertok(generic_phy_power_on(&phy2));
	ut_asserteq(-EIO, generic_phy_power_off(&phy2));

	/* PHY3 has a known problem with power off and power on */
	ut_assertok(generic_phy_init(&phy3));
	ut_asserteq(-EIO, generic_phy_power_on(&phy3));
	ut_assertok(generic_phy_power_off(&phy3));

	return 0;
}
DM_TEST(dm_test_phy_ops, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_phy_bulk(struct unit_test_state *uts)
{
	struct phy_bulk phys;
	struct udevice *parent;

	/* test normal operations */
	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user1", &parent));

	ut_assertok(generic_phy_get_bulk(parent, &phys));
	ut_asserteq(2, phys.count);

	ut_asserteq(0, generic_phy_init_bulk(&phys));
	ut_asserteq(0, generic_phy_power_on_bulk(&phys));
	ut_asserteq(0, generic_phy_power_off_bulk(&phys));
	ut_asserteq(0, generic_phy_exit_bulk(&phys));

	/* has a known problem phy */
	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user", &parent));

	ut_assertok(generic_phy_get_bulk(parent, &phys));
	ut_asserteq(3, phys.count);

	ut_asserteq(0, generic_phy_init_bulk(&phys));
	ut_asserteq(-EIO, generic_phy_power_on_bulk(&phys));
	ut_asserteq(-EIO, generic_phy_power_off_bulk(&phys));
	ut_asserteq(0, generic_phy_exit_bulk(&phys));

	return 0;
}
DM_TEST(dm_test_phy_bulk, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_phy_multi_exit(struct unit_test_state *uts)
{
	struct phy phy1_method1;
	struct phy phy1_method2;
	struct phy phy1_method3;
	struct udevice *parent;

	/* Get the same phy instance in 3 different ways. */
	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user", &parent));
	ut_assertok(generic_phy_get_by_name(parent, "phy1", &phy1_method1));
	ut_asserteq(0, phy1_method1.id);
	ut_assertok(generic_phy_get_by_name(parent, "phy1", &phy1_method2));
	ut_asserteq(0, phy1_method2.id);
	ut_asserteq_ptr(phy1_method1.dev, phy1_method1.dev);

	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user1", &parent));
	ut_assertok(generic_phy_get_by_name(parent, "phy1", &phy1_method3));
	ut_asserteq(0, phy1_method3.id);
	ut_asserteq_ptr(phy1_method1.dev, phy1_method3.dev);

	/*
	 * Test using the same PHY from different handles.
	 * In non-test code these could be in different drivers.
	 */

	/*
	 * These must only call the driver's ops at the first init()
	 * and power_on().
	 */
	ut_assertok(generic_phy_init(&phy1_method1));
	ut_assertok(generic_phy_init(&phy1_method2));
	ut_assertok(generic_phy_power_on(&phy1_method1));
	ut_assertok(generic_phy_power_on(&phy1_method2));
	ut_assertok(generic_phy_init(&phy1_method3));
	ut_assertok(generic_phy_power_on(&phy1_method3));

	/*
	 * These must not call the driver's ops as other handles still
	 * want the PHY powered-on and initialized.
	 */
	ut_assertok(generic_phy_power_off(&phy1_method3));
	ut_assertok(generic_phy_exit(&phy1_method3));

	/*
	 * We would get an error here if the generic_phy_exit() above
	 * actually called the driver's exit(), as the sandbox driver
	 * doesn't allow power-off() after exit().
	 */
	ut_assertok(generic_phy_power_off(&phy1_method1));
	ut_assertok(generic_phy_power_off(&phy1_method2));
	ut_assertok(generic_phy_exit(&phy1_method1));
	ut_assertok(generic_phy_exit(&phy1_method2));

	return 0;
}
DM_TEST(dm_test_phy_multi_exit, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_phy_setup(struct unit_test_state *uts)
{
	struct phy phy;
	struct udevice *parent;

	ut_assertok(uclass_get_device_by_name(UCLASS_SIMPLE_BUS,
					      "gen_phy_user", &parent));

	/* normal */
	ut_assertok(generic_setup_phy(parent, &phy, 0));
	ut_assertok(generic_shutdown_phy(&phy));

	/* power_off fail with -EIO */
	ut_assertok(generic_setup_phy(parent, &phy, 1));
	ut_asserteq(-EIO, generic_shutdown_phy(&phy));

	/* power_on fail with -EIO */
	ut_asserteq(-EIO, generic_setup_phy(parent, &phy, 2));
	ut_assertok(generic_shutdown_phy(&phy));

	/* generic_phy_get_by_index fail with -ENOENT */
	ut_asserteq(-ENOENT, generic_phy_get_by_index(parent, 3, &phy));
	ut_assertok(generic_setup_phy(parent, &phy, 3));
	ut_assertok(generic_shutdown_phy(&phy));

	return 0;
}
DM_TEST(dm_test_phy_setup, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
