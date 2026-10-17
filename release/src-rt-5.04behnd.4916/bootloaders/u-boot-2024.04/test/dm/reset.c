// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <log.h>
#include <malloc.h>
#include <reset.h>
#include <dm/test.h>
#include <asm/reset.h>
#include <test/test.h>
#include <test/ut.h>

/* This must match the specifier for mbox-names="test" in the DT node */
#define TEST_RESET_ID 2

/* This is the other reset phandle specifier handled by bulk */
#define OTHER_RESET_ID 2

/* Base test of the reset uclass */
static int dm_test_reset_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct reset_ctl reset_method1, reset_method1_1;
	struct reset_ctl reset_method2, reset_method2_1;
	struct reset_ctl reset_method3, reset_method3_1;
	struct reset_ctl reset_method4, reset_method4_1;

	/* Get the device using the reset device */
	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "reset-ctl-test",
					      &dev));

	/* Get the same reset port in 2 different ways and compare */
	ut_assertok(reset_get_by_index(dev, 0, &reset_method1));
	ut_assertok(reset_get_by_name(dev, NULL, &reset_method1_1));
	ut_assertok(reset_get_by_index(dev, 1, &reset_method2));
	ut_assertok(reset_get_by_index_nodev(dev_ofnode(dev), 1,
					     &reset_method2_1));
	ut_assertok(reset_get_by_index(dev, 2, &reset_method3));
	ut_assertok(reset_get_by_index_nodev(dev_ofnode(dev), 2,
					     &reset_method3_1));
	ut_assertok(reset_get_by_index(dev, 3, &reset_method4));
	ut_assertok(reset_get_by_index_nodev(dev_ofnode(dev), 3,
					     &reset_method4_1));

	ut_asserteq(reset_method1.id, reset_method1_1.id);
	ut_asserteq(reset_method2.id, reset_method2_1.id);
	ut_asserteq(reset_method3.id, reset_method3_1.id);
	ut_asserteq(reset_method4.id, reset_method4_1.id);

	ut_asserteq(true, reset_method1.id != reset_method2.id);
	ut_asserteq(true, reset_method1.id != reset_method3.id);
	ut_asserteq(true, reset_method1.id != reset_method4.id);
	ut_asserteq(true, reset_method2.id != reset_method3.id);
	ut_asserteq(true, reset_method2.id != reset_method4.id);
	ut_asserteq(true, reset_method3.id != reset_method4.id);

	ut_asserteq(true, reset_method1_1.id != reset_method2_1.id);
	ut_asserteq(true, reset_method1_1.id != reset_method3_1.id);
	ut_asserteq(true, reset_method1_1.id != reset_method4_1.id);
	ut_asserteq(true, reset_method2_1.id != reset_method3_1.id);
	ut_asserteq(true, reset_method2_1.id != reset_method4_1.id);
	ut_asserteq(true, reset_method3_1.id != reset_method4_1.id);

	return 0;
}

DM_TEST(dm_test_reset_base, UT_TESTF_SCAN_FDT);

static int dm_test_reset(struct unit_test_state *uts)
{
	struct udevice *dev_reset;
	struct udevice *dev_test;

	ut_assertok(uclass_get_device_by_name(UCLASS_RESET, "reset-ctl",
					      &dev_reset));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "reset-ctl-test",
					      &dev_test));
	ut_assertok(sandbox_reset_test_get(dev_test));

	ut_assertok(sandbox_reset_test_assert(dev_test));
	ut_asserteq(1, sandbox_reset_query(dev_reset, TEST_RESET_ID));

	ut_assertok(sandbox_reset_test_deassert(dev_test));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));

	ut_asserteq(1, sandbox_reset_is_requested(dev_reset, TEST_RESET_ID));
	ut_assertok(sandbox_reset_test_free(dev_test));
	ut_asserteq(0, sandbox_reset_is_requested(dev_reset, TEST_RESET_ID));

	return 0;
}
DM_TEST(dm_test_reset, UT_TESTF_SCAN_FDT);

static int dm_test_reset_devm(struct unit_test_state *uts)
{
	struct udevice *dev_reset;
	struct udevice *dev_test;

	ut_assertok(uclass_get_device_by_name(UCLASS_RESET, "reset-ctl",
					      &dev_reset));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "reset-ctl-test",
					      &dev_test));
	ut_assertok(sandbox_reset_test_get_devm(dev_test));

	ut_assertok(sandbox_reset_test_assert(dev_test));
	ut_asserteq(1, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_assertok(sandbox_reset_test_deassert(dev_test));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));

	ut_asserteq(1, sandbox_reset_is_requested(dev_reset, TEST_RESET_ID));
	ut_assertok(device_remove(dev_test, DM_REMOVE_NORMAL));
	ut_asserteq(0, sandbox_reset_is_requested(dev_reset, TEST_RESET_ID));

	return 0;
}
DM_TEST(dm_test_reset_devm, UT_TESTF_SCAN_FDT);

static int dm_test_reset_bulk(struct unit_test_state *uts)
{
	struct udevice *dev_reset;
	struct udevice *dev_test;

	ut_assertok(uclass_get_device_by_name(UCLASS_RESET, "reset-ctl",
					      &dev_reset));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(0, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "reset-ctl-test",
					      &dev_test));
	ut_assertok(sandbox_reset_test_get_bulk(dev_test));

	ut_assertok(sandbox_reset_test_assert_bulk(dev_test));
	ut_asserteq(1, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(1, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	ut_assertok(sandbox_reset_test_deassert_bulk(dev_test));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(0, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	ut_assertok(sandbox_reset_test_release_bulk(dev_test));
	ut_asserteq(1, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(1, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	return 0;
}
DM_TEST(dm_test_reset_bulk, UT_TESTF_SCAN_FDT);

static int dm_test_reset_bulk_devm(struct unit_test_state *uts)
{
	struct udevice *dev_reset;
	struct udevice *dev_test;

	ut_assertok(uclass_get_device_by_name(UCLASS_RESET, "reset-ctl",
					      &dev_reset));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(0, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "reset-ctl-test",
					      &dev_test));
	ut_assertok(sandbox_reset_test_get_bulk_devm(dev_test));

	ut_assertok(sandbox_reset_test_assert_bulk(dev_test));
	ut_asserteq(1, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(1, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	ut_assertok(sandbox_reset_test_deassert_bulk(dev_test));
	ut_asserteq(0, sandbox_reset_query(dev_reset, TEST_RESET_ID));
	ut_asserteq(0, sandbox_reset_query(dev_reset, OTHER_RESET_ID));

	ut_asserteq(1, sandbox_reset_is_requested(dev_reset, OTHER_RESET_ID));
	ut_asserteq(1, sandbox_reset_is_requested(dev_reset, TEST_RESET_ID));
	ut_assertok(device_remove(dev_test, DM_REMOVE_NORMAL));
	ut_asserteq(0, sandbox_reset_is_requested(dev_reset, TEST_RESET_ID));
	ut_asserteq(0, sandbox_reset_is_requested(dev_reset, OTHER_RESET_ID));

	return 0;
}
DM_TEST(dm_test_reset_bulk_devm, UT_TESTF_SCAN_FDT);
