// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <common.h>
#include <dm.h>
#include <miiphy.h>
#include <misc.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* macros copied over from mdio_sandbox.c */
#define SANDBOX_PHY_ADDR	5
#define SANDBOX_PHY_REG_CNT	2

#define TEST_REG_VALUE		0xabcd

static int dm_test_mdio_mux(struct unit_test_state *uts)
{
	struct uclass *uc;
	struct udevice *mux;
	struct udevice *mdio_ch0, *mdio_ch1, *mdio;
	struct mdio_ops *ops, *ops_parent;
	struct mdio_mux_ops *mmops;
	u16 reg;

	ut_assertok(uclass_get(UCLASS_MDIO_MUX, &uc));

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO_MUX, "mdio-mux-test",
					      &mux));

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-ch-test@0",
					      &mdio_ch0));
	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-ch-test@1",
					      &mdio_ch1));

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-test", &mdio));

	ops = mdio_get_ops(mdio_ch0);
	ut_assertnonnull(ops);
	ut_assertnonnull(ops->read);
	ut_assertnonnull(ops->write);

	mmops = mdio_mux_get_ops(mux);
	ut_assertnonnull(mmops);
	ut_assertnonnull(mmops->select);

	ops_parent = mdio_get_ops(mdio);
	ut_assertnonnull(ops);
	ut_assertnonnull(ops->read);

	/*
	 * mux driver sets last register on the emulated PHY whenever a group
	 * is selected to the selection #.  Just reading that register from
	 * either of the child buses should return the id of the child bus
	 */
	reg = dm_mdio_read(mdio_ch0, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			   SANDBOX_PHY_REG_CNT - 1);
	ut_asserteq(reg, 0);

	reg = dm_mdio_read(mdio_ch1, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			   SANDBOX_PHY_REG_CNT - 1);
	ut_asserteq(reg, 1);

	mmops->select(mux, MDIO_MUX_SELECT_NONE, 5);
	reg = dm_mdio_read(mdio, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			   SANDBOX_PHY_REG_CNT - 1);
	ut_asserteq(reg, 5);

	mmops->deselect(mux, 5);
	reg = dm_mdio_read(mdio, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			   SANDBOX_PHY_REG_CNT - 1);
	ut_asserteq(reg, (u16)MDIO_MUX_SELECT_NONE);

	return 0;
}

DM_TEST(dm_test_mdio_mux, UT_TESTF_SCAN_FDT);
