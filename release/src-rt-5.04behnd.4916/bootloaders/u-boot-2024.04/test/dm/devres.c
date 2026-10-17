// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the devres (
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/devres.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

/* Test that devm_kmalloc() allocates memory, free when device is removed */
static int dm_test_devres_alloc(struct unit_test_state *uts)
{
	ulong mem_start, mem_dev, mem_kmalloc;
	struct udevice *dev;
	void *ptr;

	mem_start = ut_check_delta(0);
	ut_assertok(uclass_first_device_err(UCLASS_TEST, &dev));
	mem_dev = ut_check_delta(mem_start);
	ut_assert(mem_dev > 0);

	/* This should increase allocated memory */
	ptr = devm_kmalloc(dev, TEST_DEVRES_SIZE, 0);
	ut_assert(ptr != NULL);
	mem_kmalloc = ut_check_delta(mem_dev);
	ut_assert(mem_kmalloc > 0);

	/* Check that ptr is freed */
	device_remove(dev, DM_REMOVE_NORMAL);
	ut_asserteq(0, ut_check_delta(mem_start));

	return 0;
}
DM_TEST(dm_test_devres_alloc, UT_TESTF_SCAN_PDATA);

/* Test devm_kfree() can be used to free memory too */
static int dm_test_devres_free(struct unit_test_state *uts)
{
	ulong mem_start, mem_dev, mem_kmalloc;
	struct udevice *dev;
	void *ptr;

	mem_start = ut_check_delta(0);
	ut_assertok(uclass_first_device_err(UCLASS_TEST, &dev));
	mem_dev = ut_check_delta(mem_start);
	ut_assert(mem_dev > 0);

	ptr = devm_kmalloc(dev, TEST_DEVRES_SIZE, 0);
	ut_assert(ptr != NULL);
	mem_kmalloc = ut_check_delta(mem_dev);
	ut_assert(mem_kmalloc > 0);

	/* Free the ptr and check that memory usage goes down */
	devm_kfree(dev, ptr);
	ut_assert(ut_check_delta(mem_kmalloc) < 0);

	device_remove(dev, DM_REMOVE_NORMAL);
	ut_asserteq(0, ut_check_delta(mem_start));

	return 0;
}
DM_TEST(dm_test_devres_free, UT_TESTF_SCAN_PDATA);


/* Test that kzalloc() returns memory that is zeroed */
static int dm_test_devres_kzalloc(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 *ptr, val;
	int i;

	ut_assertok(uclass_first_device_err(UCLASS_TEST, &dev));

	ptr = devm_kzalloc(dev, TEST_DEVRES_SIZE, 0);
	ut_assert(ptr != NULL);
	for (val = 0, i = 0; i < TEST_DEVRES_SIZE; i++)
		val |= *ptr;
	ut_asserteq(0, val);

	return 0;
}
DM_TEST(dm_test_devres_kzalloc, UT_TESTF_SCAN_PDATA);

/* Test that devm_kmalloc_array() allocates an array that can be set */
static int dm_test_devres_kmalloc_array(struct unit_test_state *uts)
{
	ulong mem_start, mem_dev;
	struct udevice *dev;
	u8 *ptr;

	mem_start = ut_check_delta(0);
	ut_assertok(uclass_first_device_err(UCLASS_TEST, &dev));
	mem_dev = ut_check_delta(mem_start);

	ptr = devm_kmalloc_array(dev, TEST_DEVRES_COUNT, TEST_DEVRES_SIZE, 0);
	ut_assert(ptr != NULL);
	memset(ptr, '\xff', TEST_DEVRES_TOTAL);
	ut_assert(ut_check_delta(mem_dev) > 0);

	device_remove(dev, DM_REMOVE_NORMAL);
	ut_asserteq(0, ut_check_delta(mem_start));

	return 0;
}
DM_TEST(dm_test_devres_kmalloc_array, UT_TESTF_SCAN_PDATA);

/* Test that devm_kcalloc() allocates a zeroed array */
static int dm_test_devres_kcalloc(struct unit_test_state *uts)
{
	ulong mem_start, mem_dev;
	struct udevice *dev;
	u8 *ptr, val;
	int i;

	mem_start = ut_check_delta(0);
	ut_assertok(uclass_first_device_err(UCLASS_TEST, &dev));
	mem_dev = ut_check_delta(mem_start);
	ut_assert(mem_dev > 0);

	/* This should increase allocated memory */
	ptr = devm_kcalloc(dev, TEST_DEVRES_SIZE, TEST_DEVRES_COUNT, 0);
	ut_assert(ptr != NULL);
	ut_assert(ut_check_delta(mem_dev) > 0);
	for (val = 0, i = 0; i < TEST_DEVRES_TOTAL; i++)
		val |= *ptr;
	ut_asserteq(0, val);

	/* Check that ptr is freed */
	device_remove(dev, DM_REMOVE_NORMAL);
	ut_asserteq(0, ut_check_delta(mem_start));

	return 0;
}
DM_TEST(dm_test_devres_kcalloc, UT_TESTF_SCAN_PDATA);

/* Test devres releases resources automatically as expected */
static int dm_test_devres_phase(struct unit_test_state *uts)
{
	struct devres_stats stats;
	struct udevice *dev;

	/*
	 * The device is bound already, so find it and check that it has the
	 * allocation created in the bind() method.
	 */
	ut_assertok(uclass_find_first_device(UCLASS_TEST_DEVRES, &dev));
	ut_assertnonnull(dev);
	devres_get_stats(dev, &stats);
	ut_asserteq(1, stats.allocs);
	ut_asserteq(TEST_DEVRES_SIZE, stats.total_size);

	/* Getting plat should add one allocation */
	ut_assertok(device_of_to_plat(dev));
	devres_get_stats(dev, &stats);
	ut_asserteq(2, stats.allocs);
	ut_asserteq(TEST_DEVRES_SIZE + TEST_DEVRES_SIZE3, stats.total_size);

	/* Probing the device should add one allocation */
	ut_assertok(uclass_first_device_err(UCLASS_TEST_DEVRES, &dev));
	ut_assertnonnull(dev);
	devres_get_stats(dev, &stats);
	ut_asserteq(3, stats.allocs);
	ut_asserteq(TEST_DEVRES_SIZE + TEST_DEVRES_SIZE2 + TEST_DEVRES_SIZE3,
		    stats.total_size);

	/* Removing the device should drop both those allocations */
	device_remove(dev, DM_REMOVE_NORMAL);
	devres_get_stats(dev, &stats);
	ut_asserteq(1, stats.allocs);
	ut_asserteq(TEST_DEVRES_SIZE, stats.total_size);

	/* Unbinding removes the other. */
	device_unbind(dev);

	return 0;
}
DM_TEST(dm_test_devres_phase, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
