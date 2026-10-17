// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - https://www.ti.com/
 * Jean-Jacques Hiblot <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <mux.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/test.h>
#include <dm/test.h>
#include <dm/device-internal.h>
#include <test/ut.h>

static int dm_test_mux_mmio_select(struct unit_test_state *uts)
{
	struct udevice *dev, *dev_b;
	struct regmap *map;
	struct mux_control *ctl0_a, *ctl0_b;
	struct mux_control *ctl1;
	struct mux_control *ctl_err;
	u32 val;
	int i;

	sandbox_set_enable_memio(true);

	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "a-test",
					      &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "b-test",
					      &dev_b));
	map = syscon_regmap_lookup_by_phandle(dev, "mux-syscon");
	ut_assertok_ptr(map);
	ut_assert(map);

	ut_assertok(mux_control_get(dev, "mux0", &ctl0_a));
	ut_assertok(mux_control_get(dev, "mux1", &ctl1));
	ut_asserteq(-ERANGE, mux_control_get(dev, "mux3", &ctl_err));
	ut_asserteq(-ENODATA, mux_control_get(dev, "dummy", &ctl_err));
	ut_assertok(mux_control_get(dev_b, "mux0", &ctl0_b));

	for (i = 0; i < mux_control_states(ctl0_a); i++) {
		/* Select a new state and verify the value in the regmap. */
		ut_assertok(mux_control_select(ctl0_a, i));
		ut_assertok(regmap_read(map, 0, &val));
		ut_asserteq(i, (val & 0x30) >> 4);
		/*
		 * Deselect the mux and verify that the value in the regmap
		 * reflects the idle state (fixed to MUX_IDLE_AS_IS).
		 */
		ut_assertok(mux_control_deselect(ctl0_a));
		ut_assertok(regmap_read(map, 0, &val));
		ut_asserteq(i, (val & 0x30) >> 4);
	}

	for (i = 0; i < mux_control_states(ctl1); i++) {
		/* Select a new state and verify the value in the regmap. */
		ut_assertok(mux_control_select(ctl1, i));
		ut_assertok(regmap_read(map, 0xc, &val));
		ut_asserteq(i, (val & 0x1E) >> 1);
		/*
		 * Deselect the mux and verify that the value in the regmap
		 * reflects the idle state (fixed to 2).
		 */
		ut_assertok(mux_control_deselect(ctl1));
		ut_assertok(regmap_read(map, 0xc, &val));
		ut_asserteq(2, (val & 0x1E) >> 1);
	}

	/* Try unbalanced selection/deselection. */
	ut_assertok(mux_control_select(ctl0_a, 0));
	ut_asserteq(-EBUSY, mux_control_select(ctl0_a, 1));
	ut_asserteq(-EBUSY, mux_control_select(ctl0_a, 0));
	ut_assertok(mux_control_deselect(ctl0_a));

	/* Try concurrent selection. */
	ut_assertok(mux_control_select(ctl0_a, 0));
	ut_assert(mux_control_select(ctl0_b, 0));
	ut_assertok(mux_control_deselect(ctl0_a));
	ut_assertok(mux_control_select(ctl0_b, 0));
	ut_assert(mux_control_select(ctl0_a, 0));
	ut_assertok(mux_control_deselect(ctl0_b));
	ut_assertok(mux_control_select(ctl0_a, 0));
	ut_assertok(mux_control_deselect(ctl0_a));

	return 0;
}
DM_TEST(dm_test_mux_mmio_select, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that managed API for mux work correctly */
static int dm_test_devm_mux_mmio(struct unit_test_state *uts)
{
	struct udevice *dev, *dev_b;
	struct mux_control *ctl0_a, *ctl0_b;
	struct mux_control *ctl1;
	struct mux_control *ctl_err;

	sandbox_set_enable_memio(true);

	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "a-test",
					      &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "b-test",
					      &dev_b));

	ctl0_a = devm_mux_control_get(dev, "mux0");
	ut_assertok_ptr(ctl0_a);
	ut_assert(ctl0_a);
	ctl1 = devm_mux_control_get(dev, "mux1");
	ut_assertok_ptr(ctl1);
	ut_assert(ctl1);
	ctl_err = devm_mux_control_get(dev, "mux3");
	ut_asserteq(-ERANGE, PTR_ERR(ctl_err));
	ctl_err = devm_mux_control_get(dev, "dummy");
	ut_asserteq(-ENODATA, PTR_ERR(ctl_err));

	ctl0_b = devm_mux_control_get(dev_b, "mux0");
	ut_assertok_ptr(ctl0_b);
	ut_assert(ctl0_b);

	/* Try concurrent selection. */
	ut_assertok(mux_control_select(ctl0_a, 0));
	ut_assert(mux_control_select(ctl0_b, 0));
	ut_assertok(mux_control_deselect(ctl0_a));
	ut_assertok(mux_control_select(ctl0_b, 0));
	ut_assert(mux_control_select(ctl0_a, 0));
	ut_assertok(mux_control_deselect(ctl0_b));

	/* Remove one device and check that the mux is released. */
	ut_assertok(mux_control_select(ctl0_a, 0));
	ut_assert(mux_control_select(ctl0_b, 0));
	device_remove(dev, DM_REMOVE_NORMAL);
	ut_assertok(mux_control_select(ctl0_b, 0));

	device_remove(dev_b, DM_REMOVE_NORMAL);
	return 0;
}
DM_TEST(dm_test_devm_mux_mmio, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
