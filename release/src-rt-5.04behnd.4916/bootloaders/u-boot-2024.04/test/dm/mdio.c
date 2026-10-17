// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Alex Marginean, NXP
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <miiphy.h>
#include <misc.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* macros copied over from mdio_sandbox.c */
#define SANDBOX_PHY_ADDR	5
#define SANDBOX_PHY_REG_CNT	2

/* test using 1st register, 0 */
#define SANDBOX_PHY_REG		0

#define TEST_REG_VALUE		0xabcd

static int dm_test_mdio(struct unit_test_state *uts)
{
	struct uclass *uc;
	struct udevice *dev;
	struct mdio_ops *ops;
	u16 reg;

	ut_assertok(uclass_get(UCLASS_MDIO, &uc));

	ut_assertok(uclass_get_device_by_name(UCLASS_MDIO, "mdio-test", &dev));

	ops = mdio_get_ops(dev);
	ut_assertnonnull(ops);
	ut_assertnonnull(ops->read);
	ut_assertnonnull(ops->write);

	ut_assertok(dm_mdio_write(dev, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
				  SANDBOX_PHY_REG, TEST_REG_VALUE));
	reg = dm_mdio_read(dev, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			   SANDBOX_PHY_REG);
	ut_asserteq(reg, TEST_REG_VALUE);

	ut_assert(dm_mdio_read(dev, SANDBOX_PHY_ADDR + 1, MDIO_DEVAD_NONE,
			       SANDBOX_PHY_REG) != 0);

	ut_assertok(dm_mdio_reset(dev));
	reg = dm_mdio_read(dev, SANDBOX_PHY_ADDR, MDIO_DEVAD_NONE,
			   SANDBOX_PHY_REG);
	ut_asserteq(reg, 0);

	return 0;
}

DM_TEST(dm_test_mdio, UT_TESTF_SCAN_FDT);
