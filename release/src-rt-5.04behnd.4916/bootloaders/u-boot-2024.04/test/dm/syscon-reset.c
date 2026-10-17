// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <test/ut.h>
#include <asm/test.h>
#include <linux/bitops.h>

/* The following values must match the device tree */
#define TEST_RESET_REG 1
#define TEST_RESET_ASSERT_HIGH 0
#define TEST_RESET_ASSERT (TEST_RESET_ASSERT_HIGH ? (u32)-1 : (u32)0)
#define TEST_RESET_DEASSERT (~TEST_RESET_ASSERT)

#define TEST_RESET_VALID 15
#define TEST_RESET_NOMASK 30
#define TEST_RESET_OUTOFRANGE 60

static int dm_test_syscon_reset(struct unit_test_state *uts)
{
	struct regmap *map;
	struct reset_ctl rst;
	struct udevice *reset;
	struct udevice *syscon;
	struct udevice *syscon_reset;
	uint reg;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "syscon-reset-test",
					      &reset));
	ut_assertok(uclass_get_device_by_name(UCLASS_SYSCON, "syscon@0",
					      &syscon));
	ut_assertok(uclass_get_device_by_name(UCLASS_RESET, "syscon-reset",
					      &syscon_reset));
	ut_assertok_ptr((map = syscon_get_regmap(syscon)));

	ut_asserteq(-EINVAL, reset_get_by_name(reset, "no_mask", &rst));
	ut_asserteq(-EINVAL, reset_get_by_name(reset, "out_of_range", &rst));
	ut_assertok(reset_get_by_name(reset, "valid", &rst));

	sandbox_set_enable_memio(true);
	ut_assertok(regmap_write(map, TEST_RESET_REG, TEST_RESET_DEASSERT));
	ut_assertok(reset_assert(&rst));
	ut_assertok(regmap_read(map, TEST_RESET_REG, &reg));
	ut_asserteq(TEST_RESET_DEASSERT ^ BIT(TEST_RESET_VALID), reg);

	ut_assertok(reset_deassert(&rst));
	ut_assertok(regmap_read(map, TEST_RESET_REG, &reg));
	ut_asserteq(TEST_RESET_DEASSERT, reg);

	return 0;
}
DM_TEST(dm_test_syscon_reset, UT_TESTF_SCAN_FDT);
