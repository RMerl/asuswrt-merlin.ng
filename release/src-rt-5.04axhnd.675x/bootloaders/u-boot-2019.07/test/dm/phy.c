// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <dm/test.h>
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
	ut_assertok(generic_phy_get_by_name(parent, "phy1", &phy1_method1))
	ut_assertok(generic_phy_get_by_index(parent, 0, &phy1_method2))
	ut_asserteq(phy1_method1.id, phy1_method2.id);

	/*
	 * Get the second phy port. Check that the same phy provider (device)
	 * provides this 2nd phy port, but that the IDs are different
	 */
	ut_assertok(generic_phy_get_by_name(parent, "phy2", &phy2))
	ut_asserteq_ptr(phy1_method2.dev, phy2.dev);
	ut_assert(phy1_method1.id != phy2.id);

	/*
	 * Get the third phy port. Check that the phy provider is different
	 */
	ut_assertok(generic_phy_get_by_name(parent, "phy3", &phy3))
	ut_assert(phy2.dev != phy3.dev);

	/* Try to get a non-existing phy */
	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_PHY, 3, &dev));
	ut_asserteq(-ENODATA, generic_phy_get_by_name(parent,
					"phy_not_existing", &phy1_method1));

	return 0;
}
DM_TEST(dm_test_phy_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

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
	 * test operations after exit().
	 * The sandbox phy driver does not allow it.
	 */
	ut_assertok(generic_phy_exit(&phy1));
	ut_assert(generic_phy_power_on(&phy1) != 0);
	ut_assert(generic_phy_power_off(&phy1) != 0);

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

	/* PHY2 has a known problem with power off */
	ut_assertok(generic_phy_init(&phy2));
	ut_assertok(generic_phy_power_on(&phy2));
	ut_asserteq(-EIO, generic_phy_power_off(&phy2));

	/* PHY3 has a known problem with power off and power on */
	ut_assertok(generic_phy_init(&phy3));
	ut_asserteq(-EIO, generic_phy_power_off(&phy3));
	ut_asserteq(-EIO, generic_phy_power_off(&phy3));

	return 0;
}
DM_TEST(dm_test_phy_ops, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
