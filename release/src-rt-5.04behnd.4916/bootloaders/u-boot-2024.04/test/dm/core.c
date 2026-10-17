// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the core driver model code
 *
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	TEST_INTVAL1		= 0,
	TEST_INTVAL2		= 3,
	TEST_INTVAL3		= 6,
	TEST_INTVAL_MANUAL	= 101112,
	TEST_INTVAL_PRE_RELOC	= 7,
};

static const struct dm_test_pdata test_pdata[] = {
	{ .ping_add		= TEST_INTVAL1, },
	{ .ping_add		= TEST_INTVAL2, },
	{ .ping_add		= TEST_INTVAL3, },
};

static const struct dm_test_pdata test_pdata_manual = {
	.ping_add		= TEST_INTVAL_MANUAL,
};

static const struct dm_test_pdata test_pdata_pre_reloc = {
	.ping_add		= TEST_INTVAL_PRE_RELOC,
};

U_BOOT_DRVINFO(dm_test_info1) = {
	.name = "test_drv",
	.plat = &test_pdata[0],
};

U_BOOT_DRVINFO(dm_test_info2) = {
	.name = "test_drv",
	.plat = &test_pdata[1],
};

U_BOOT_DRVINFO(dm_test_info3) = {
	.name = "test_drv",
	.plat = &test_pdata[2],
};

static struct driver_info driver_info_manual = {
	.name = "test_manual_drv",
	.plat = &test_pdata_manual,
};

static struct driver_info driver_info_pre_reloc = {
	.name = "test_pre_reloc_drv",
	.plat = &test_pdata_pre_reloc,
};

static struct driver_info driver_info_act_dma = {
	.name = "test_act_dma_drv",
};

static struct driver_info driver_info_vital_clk = {
	.name = "test_vital_clk_drv",
};

static struct driver_info driver_info_act_dma_vital_clk = {
	.name = "test_act_dma_vital_clk_drv",
};

void dm_leak_check_start(struct unit_test_state *uts)
{
	uts->start = mallinfo();
	if (!uts->start.uordblks)
		puts("Warning: Please add '#define DEBUG' to the top of common/dlmalloc.c\n");
}

int dm_leak_check_end(struct unit_test_state *uts)
{
	struct mallinfo end;
	int id, diff;

	/* Don't delete the root class, since we started with that */
	for (id = UCLASS_ROOT + 1; id < UCLASS_COUNT; id++) {
		struct uclass *uc;

		uc = uclass_find(id);
		if (!uc)
			continue;
		ut_assertok(uclass_destroy(uc));
	}

	end = mallinfo();
	diff = end.uordblks - uts->start.uordblks;
	if (diff > 0)
		printf("Leak: lost %#xd bytes\n", diff);
	else if (diff < 0)
		printf("Leak: gained %#xd bytes\n", -diff);
	ut_asserteq(uts->start.uordblks, end.uordblks);

	return 0;
}

/* Test that binding with plat occurs correctly */
static int dm_test_autobind(struct unit_test_state *uts)
{
	struct udevice *dev;

	/*
	 * We should have a single class (UCLASS_ROOT) and a single root
	 * device with no children.
	 */
	ut_assert(uts->root);
	ut_asserteq(1, list_count_items(gd->uclass_root));
	ut_asserteq(0, list_count_items(&gd->dm_root->child_head));
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_POST_BIND]);

	ut_assertok(dm_scan_plat(false));

	/* We should have our test class now at least, plus more children */
	ut_assert(1 < list_count_items(gd->uclass_root));
	ut_assert(0 < list_count_items(&gd->dm_root->child_head));

	/* Our 3 dm_test_infox children should be bound to the test uclass */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_POST_BIND]);

	/* No devices should be probed */
	list_for_each_entry(dev, &gd->dm_root->child_head, sibling_node)
		ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	/* Our test driver should have been bound 3 times */
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_BIND] == 3);

	return 0;
}
DM_TEST(dm_test_autobind, 0);

/* Test that binding with uclass plat allocation occurs correctly */
static int dm_test_autobind_uclass_pdata_alloc(struct unit_test_state *uts)
{
	struct dm_test_perdev_uc_pdata *uc_pdata;
	struct udevice *dev;
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_assert(uc);

	/**
	 * Test if test uclass driver requires allocation for the uclass
	 * platform data and then check the dev->uclass_plat pointer.
	 */
	ut_assert(uc->uc_drv->per_device_plat_auto);

	for (uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		ut_assertnonnull(dev);

		uc_pdata = dev_get_uclass_plat(dev);
		ut_assert(uc_pdata);
	}

	return 0;
}
DM_TEST(dm_test_autobind_uclass_pdata_alloc, UT_TESTF_SCAN_PDATA);

/* compare node names ignoring the unit address */
static int dm_test_compare_node_name(struct unit_test_state *uts)
{
	ofnode node;

	node = ofnode_path("/mmio-bus@0");
	ut_assert(ofnode_valid(node));
	ut_assert(ofnode_name_eq(node, "mmio-bus"));

	return 0;
}

DM_TEST(dm_test_compare_node_name, UT_TESTF_SCAN_PDATA);

/* Test that binding with uclass plat setting occurs correctly */
static int dm_test_autobind_uclass_pdata_valid(struct unit_test_state *uts)
{
	struct dm_test_perdev_uc_pdata *uc_pdata;
	struct udevice *dev;

	/**
	 * In the test_postbind() method of test uclass driver, the uclass
	 * platform data should be set to three test int values - test it.
	 */
	for (uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		ut_assertnonnull(dev);

		uc_pdata = dev_get_uclass_plat(dev);
		ut_assert(uc_pdata);
		ut_assert(uc_pdata->intval1 == TEST_UC_PDATA_INTVAL1);
		ut_assert(uc_pdata->intval2 == TEST_UC_PDATA_INTVAL2);
		ut_assert(uc_pdata->intval3 == TEST_UC_PDATA_INTVAL3);
	}

	return 0;
}
DM_TEST(dm_test_autobind_uclass_pdata_valid, UT_TESTF_SCAN_PDATA);

/* Test that autoprobe finds all the expected devices */
static int dm_test_autoprobe(struct unit_test_state *uts)
{
	int expected_base_add;
	struct udevice *dev;
	struct uclass *uc;
	int i;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_assert(uc);

	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_PRE_PROBE]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_POST_PROBE]);

	/* The root device should not be activated until needed */
	ut_assert(dev_get_flags(uts->root) & DM_FLAG_ACTIVATED);

	/*
	 * We should be able to find the three test devices, and they should
	 * all be activated as they are used (lazy activation, required by
	 * U-Boot)
	 */
	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assertf(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED),
			   "Driver %d/%s already activated", i, dev->name);

		/* This should activate it */
		ut_assertok(uclass_get_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);

		/* Activating a device should activate the root device */
		if (!i)
			ut_assert(dev_get_flags(uts->root) & DM_FLAG_ACTIVATED);
	}

	/*
	 * Our 3 dm_test_info children should be passed to pre_probe and
	 * post_probe
	 */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_POST_PROBE]);
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_PRE_PROBE]);

	/* Also we can check the per-device data */
	expected_base_add = 0;
	for (i = 0; i < 3; i++) {
		struct dm_test_uclass_perdev_priv *priv;
		struct dm_test_pdata *pdata;

		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);

		priv = dev_get_uclass_priv(dev);
		ut_assert(priv);
		ut_asserteq(expected_base_add, priv->base_add);

		pdata = dev_get_plat(dev);
		expected_base_add += pdata->ping_add;
	}

	return 0;
}
DM_TEST(dm_test_autoprobe, UT_TESTF_SCAN_PDATA);

/* Check that we see the correct plat in each device */
static int dm_test_plat(struct unit_test_state *uts)
{
	const struct dm_test_pdata *pdata;
	struct udevice *dev;
	int i;

	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		pdata = dev_get_plat(dev);
		ut_assert(pdata->ping_add == test_pdata[i].ping_add);
	}

	return 0;
}
DM_TEST(dm_test_plat, UT_TESTF_SCAN_PDATA);

/* Test that we can bind, probe, remove, unbind a driver */
static int dm_test_lifecycle(struct unit_test_state *uts)
{
	int op_count[DM_TEST_OP_COUNT];
	struct udevice *dev, *test_dev;
	int start_dev_count, start_uc_count;
	int dev_count, uc_count;
	int pingret;
	int ret;

	memcpy(op_count, dm_testdrv_op_count, sizeof(op_count));

	dm_get_stats(&start_dev_count, &start_uc_count);

	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_BIND]
			== op_count[DM_TEST_OP_BIND] + 1);
	ut_assert(!dev_get_priv(dev));

	/* We should have one more device */
	dm_get_stats(&dev_count, &uc_count);
	ut_asserteq(start_dev_count + 1, dev_count);
	ut_asserteq(start_uc_count, uc_count);

	/* Probe the device - it should fail allocating private data */
	uts->force_fail_alloc = 1;
	ret = device_probe(dev);
	ut_assert(ret == -ENOMEM);
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_PROBE]
			== op_count[DM_TEST_OP_PROBE] + 1);
	ut_assert(!dev_get_priv(dev));

	/* Try again without the alloc failure */
	uts->force_fail_alloc = 0;
	ut_assertok(device_probe(dev));
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_PROBE]
			== op_count[DM_TEST_OP_PROBE] + 2);
	ut_assert(dev_get_priv(dev));

	/* This should be device 3 in the uclass */
	ut_assertok(uclass_find_device(UCLASS_TEST, 3, &test_dev));
	ut_assert(dev == test_dev);

	/* Try ping */
	ut_assertok(test_ping(dev, 100, &pingret));
	ut_assert(pingret == 102);

	/* Now remove device 3 */
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_PRE_REMOVE]);
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_PRE_REMOVE]);

	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_UNBIND]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_PRE_UNBIND]);
	ut_assertok(device_unbind(dev));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_UNBIND]);
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_PRE_UNBIND]);

	/* We should have one less device */
	dm_get_stats(&dev_count, &uc_count);
	ut_asserteq(start_dev_count, dev_count);
	ut_asserteq(start_uc_count, uc_count);

	return 0;
}
DM_TEST(dm_test_lifecycle, UT_TESTF_SCAN_PDATA | UT_TESTF_PROBE_TEST);

/* Test that we can bind/unbind and the lists update correctly */
static int dm_test_ordering(struct unit_test_state *uts)
{
	struct udevice *dev, *dev_penultimate, *dev_last, *test_dev;
	int pingret;

	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);

	/* Bind two new devices (numbers 4 and 5) */
	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&dev_penultimate));
	ut_assert(dev_penultimate);
	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&dev_last));
	ut_assert(dev_last);

	/* Now remove device 3 */
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
	ut_assertok(device_unbind(dev));

	/* The device numbering should have shifted down one */
	ut_assertok(uclass_find_device(UCLASS_TEST, 3, &test_dev));
	ut_assert(dev_penultimate == test_dev);
	ut_assertok(uclass_find_device(UCLASS_TEST, 4, &test_dev));
	ut_assert(dev_last == test_dev);

	/* Add back the original device 3, now in position 5 */
	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);

	/* Try ping */
	ut_assertok(test_ping(dev, 100, &pingret));
	ut_assert(pingret == 102);

	/* Remove 3 and 4 */
	ut_assertok(device_remove(dev_penultimate, DM_REMOVE_NORMAL));
	ut_assertok(device_unbind(dev_penultimate));
	ut_assertok(device_remove(dev_last, DM_REMOVE_NORMAL));
	ut_assertok(device_unbind(dev_last));

	/* Our device should now be in position 3 */
	ut_assertok(uclass_find_device(UCLASS_TEST, 3, &test_dev));
	ut_assert(dev == test_dev);

	/* Now remove device 3 */
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
	ut_assertok(device_unbind(dev));

	return 0;
}
DM_TEST(dm_test_ordering, UT_TESTF_SCAN_PDATA);

/* Check that we can perform operations on a device (do a ping) */
int dm_check_operations(struct unit_test_state *uts, struct udevice *dev,
			uint32_t base, struct dm_test_priv *priv)
{
	int expected;
	int pingret;

	/* Getting the child device should allocate plat / priv */
	ut_assertok(testfdt_ping(dev, 10, &pingret));
	ut_assert(dev_get_priv(dev));
	ut_assert(dev_get_plat(dev));

	expected = 10 + base;
	ut_asserteq(expected, pingret);

	/* Do another ping */
	ut_assertok(testfdt_ping(dev, 20, &pingret));
	expected = 20 + base;
	ut_asserteq(expected, pingret);

	/* Now check the ping_total */
	priv = dev_get_priv(dev);
	ut_asserteq(DM_TEST_START_TOTAL + 10 + 20 + base * 2,
		    priv->ping_total);

	return 0;
}

/* Check that we can perform operations on devices */
static int dm_test_operations(struct unit_test_state *uts)
{
	struct udevice *dev;
	int i;

	/*
	 * Now check that the ping adds are what we expect. This is using the
	 * ping-add property in each node.
	 */
	for (i = 0; i < ARRAY_SIZE(test_pdata); i++) {
		uint32_t base;

		ut_assertok(uclass_get_device(UCLASS_TEST, i, &dev));

		/*
		 * Get the 'reg' property, which tells us what the ping add
		 * should be. We don't use the plat because we want
		 * to test the code that sets that up (testfdt_drv_probe()).
		 */
		base = test_pdata[i].ping_add;
		debug("dev=%d, base=%d\n", i, base);

		ut_assert(!dm_check_operations(uts, dev, base, dev_get_priv(dev)));
	}

	return 0;
}
DM_TEST(dm_test_operations, UT_TESTF_SCAN_PDATA);

/* Remove all drivers and check that things work */
static int dm_test_remove(struct unit_test_state *uts)
{
	struct udevice *dev;
	int i;

	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assertf(dev_get_flags(dev) & DM_FLAG_ACTIVATED,
			   "Driver %d/%s not activated", i, dev->name);
		ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
		ut_assertf(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED),
			   "Driver %d/%s should have deactivated", i,
			   dev->name);
		ut_assert(!dev_get_priv(dev));
	}

	return 0;
}
DM_TEST(dm_test_remove, UT_TESTF_SCAN_PDATA | UT_TESTF_PROBE_TEST);

/* Remove and recreate everything, check for memory leaks */
static int dm_test_leak(struct unit_test_state *uts)
{
	int i;

	for (i = 0; i < 2; i++) {
		int ret;

		dm_leak_check_start(uts);

		ut_assertok(dm_scan_plat(false));
		ut_assertok(dm_scan_fdt(false));

		ret = uclass_probe_all(UCLASS_TEST);
		ut_assertok(ret);

		ut_assertok(dm_leak_check_end(uts));
	}

	return 0;
}
DM_TEST(dm_test_leak, 0);

/* Test uclass init/destroy methods */
static int dm_test_uclass(struct unit_test_state *uts)
{
	int dev_count, uc_count;
	struct uclass *uc;

	/* We should have just the root device and uclass */
	dm_get_stats(&dev_count, &uc_count);
	ut_asserteq(1, dev_count);
	ut_asserteq(1, uc_count);

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_DESTROY]);
	ut_assert(uclass_get_priv(uc));

	dm_get_stats(&dev_count, &uc_count);
	ut_asserteq(1, dev_count);
	ut_asserteq(2, uc_count);

	ut_assertok(uclass_destroy(uc));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_DESTROY]);

	dm_get_stats(&dev_count, &uc_count);
	ut_asserteq(1, dev_count);
	ut_asserteq(1, uc_count);

	return 0;
}
DM_TEST(dm_test_uclass, 0);

/**
 * create_children() - Create children of a parent node
 *
 * @dms:	Test system state
 * @parent:	Parent device
 * @count:	Number of children to create
 * @key:	Key value to put in first child. Subsequence children
 *		receive an incrementing value
 * @child:	If not NULL, then the child device pointers are written into
 *		this array.
 * Return: 0 if OK, -ve on error
 */
static int create_children(struct unit_test_state *uts, struct udevice *parent,
			   int count, int key, struct udevice *child[])
{
	struct udevice *dev;
	int i;

	for (i = 0; i < count; i++) {
		struct dm_test_pdata *pdata;

		ut_assertok(device_bind_by_name(parent, false,
						&driver_info_manual, &dev));
		pdata = calloc(1, sizeof(*pdata));
		pdata->ping_add = key + i;
		dev_set_plat(dev, pdata);
		if (child)
			child[i] = dev;
	}

	return 0;
}

#define NODE_COUNT	10

static int dm_test_children(struct unit_test_state *uts)
{
	struct udevice *top[NODE_COUNT];
	struct udevice *child[NODE_COUNT];
	struct udevice *grandchild[NODE_COUNT];
	struct udevice *dev;
	int total;
	int ret;
	int i;

	/* We don't care about the numbering for this test */
	uts->skip_post_probe = 1;

	ut_assert(NODE_COUNT > 5);

	/* First create 10 top-level children */
	ut_assertok(create_children(uts, uts->root, NODE_COUNT, 0, top));

	/* Now a few have their own children */
	ut_assertok(create_children(uts, top[2], NODE_COUNT, 2, NULL));
	ut_assertok(create_children(uts, top[5], NODE_COUNT, 5, child));

	/* And grandchildren */
	for (i = 0; i < NODE_COUNT; i++)
		ut_assertok(create_children(uts, child[i], NODE_COUNT, 50 * i,
					    i == 2 ? grandchild : NULL));

	/* Check total number of devices */
	total = NODE_COUNT * (3 + NODE_COUNT);
	ut_asserteq(total, dm_testdrv_op_count[DM_TEST_OP_BIND]);

	/* Try probing one of the grandchildren */
	ut_assertok(uclass_get_device(UCLASS_TEST,
				      NODE_COUNT * 3 + 2 * NODE_COUNT, &dev));
	ut_asserteq_ptr(grandchild[0], dev);

	/*
	 * This should have probed the child and top node also, for a total
	 * of 3 nodes.
	 */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_PROBE]);

	/* Probe the other grandchildren */
	for (i = 1; i < NODE_COUNT; i++)
		ut_assertok(device_probe(grandchild[i]));

	ut_asserteq(2 + NODE_COUNT, dm_testdrv_op_count[DM_TEST_OP_PROBE]);

	/* Probe everything */
	ret = uclass_probe_all(UCLASS_TEST);
	ut_assertok(ret);

	ut_asserteq(total, dm_testdrv_op_count[DM_TEST_OP_PROBE]);

	/* Remove a top-level child and check that the children are removed */
	ut_assertok(device_remove(top[2], DM_REMOVE_NORMAL));
	ut_asserteq(NODE_COUNT + 1, dm_testdrv_op_count[DM_TEST_OP_REMOVE]);
	dm_testdrv_op_count[DM_TEST_OP_REMOVE] = 0;

	/* Try one with grandchildren */
	ut_assertok(uclass_get_device(UCLASS_TEST, 5, &dev));
	ut_asserteq_ptr(dev, top[5]);
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
	ut_asserteq(1 + NODE_COUNT * (1 + NODE_COUNT),
		    dm_testdrv_op_count[DM_TEST_OP_REMOVE]);

	/* Try the same with unbind */
	ut_assertok(device_unbind(top[2]));
	ut_asserteq(NODE_COUNT + 1, dm_testdrv_op_count[DM_TEST_OP_UNBIND]);
	dm_testdrv_op_count[DM_TEST_OP_UNBIND] = 0;

	/* Try one with grandchildren */
	ut_assertok(uclass_get_device(UCLASS_TEST, 5, &dev));
	ut_asserteq_ptr(dev, top[6]);
	ut_assertok(device_unbind(top[5]));
	ut_asserteq(1 + NODE_COUNT * (1 + NODE_COUNT),
		    dm_testdrv_op_count[DM_TEST_OP_UNBIND]);

	return 0;
}
DM_TEST(dm_test_children, 0);

static int dm_test_device_reparent(struct unit_test_state *uts)
{
	struct udevice *top[NODE_COUNT];
	struct udevice *child[NODE_COUNT];
	struct udevice *grandchild[NODE_COUNT];
	struct udevice *dev;
	int total;
	int ret;
	int i;

	/* We don't care about the numbering for this test */
	uts->skip_post_probe = 1;

	ut_assert(NODE_COUNT > 5);

	/* First create 10 top-level children */
	ut_assertok(create_children(uts, uts->root, NODE_COUNT, 0, top));

	/* Now a few have their own children */
	ut_assertok(create_children(uts, top[2], NODE_COUNT, 2, NULL));
	ut_assertok(create_children(uts, top[5], NODE_COUNT, 5, child));

	/* And grandchildren */
	for (i = 0; i < NODE_COUNT; i++)
		ut_assertok(create_children(uts, child[i], NODE_COUNT, 50 * i,
					    i == 2 ? grandchild : NULL));

	/* Check total number of devices */
	total = NODE_COUNT * (3 + NODE_COUNT);
	ut_asserteq(total, dm_testdrv_op_count[DM_TEST_OP_BIND]);

	/* Probe everything */
	for (i = 0; i < total; i++)
		ut_assertok(uclass_get_device(UCLASS_TEST, i, &dev));

	/* Re-parent top-level children with no grandchildren. */
	ut_assertok(device_reparent(top[3], top[0]));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_reparent(top[4], top[0]));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	/* Re-parent top-level children with grandchildren. */
	ut_assertok(device_reparent(top[2], top[0]));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_reparent(top[5], top[2]));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	/* Re-parent grandchildren. */
	ut_assertok(device_reparent(grandchild[0], top[1]));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_reparent(grandchild[1], top[1]));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	/* Remove re-pareneted devices. */
	ut_assertok(device_remove(top[3], DM_REMOVE_NORMAL));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_remove(top[4], DM_REMOVE_NORMAL));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_remove(top[5], DM_REMOVE_NORMAL));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_remove(top[2], DM_REMOVE_NORMAL));
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_remove(grandchild[0], DM_REMOVE_NORMAL));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(device_remove(grandchild[1], DM_REMOVE_NORMAL));
	/* try to get devices */
	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	/* Try the same with unbind */
	ut_assertok(device_unbind(top[3]));
	ut_assertok(device_unbind(top[4]));
	ut_assertok(device_unbind(top[5]));
	ut_assertok(device_unbind(top[2]));

	ut_assertok(device_unbind(grandchild[0]));
	ut_assertok(device_unbind(grandchild[1]));

	return 0;
}
DM_TEST(dm_test_device_reparent, 0);

/* Test that pre-relocation devices work as expected */
static int dm_test_pre_reloc(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* The normal driver should refuse to bind before relocation */
	ut_asserteq(-EPERM, device_bind_by_name(uts->root, true,
						&driver_info_manual, &dev));

	/* But this one is marked pre-reloc */
	ut_assertok(device_bind_by_name(uts->root, true,
					&driver_info_pre_reloc, &dev));

	return 0;
}
DM_TEST(dm_test_pre_reloc, 0);

/*
 * Test that removal of devices, either via the "normal" device_remove()
 * API or via the device driver selective flag works as expected
 */
static int dm_test_remove_active_dma(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_act_dma,
					&dev));
	ut_assert(dev);

	/* Probe the device */
	ut_assertok(device_probe(dev));

	/* Test if device is active right now */
	ut_asserteq(true, device_active(dev));

	/* Remove the device via selective remove flag */
	dm_remove_devices_flags(DM_REMOVE_ACTIVE_ALL);

	/* Test if device is inactive right now */
	ut_asserteq(false, device_active(dev));

	/* Probe the device again */
	ut_assertok(device_probe(dev));

	/* Test if device is active right now */
	ut_asserteq(true, device_active(dev));

	/* Remove the device via "normal" remove API */
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));

	/* Test if device is inactive right now */
	ut_asserteq(false, device_active(dev));

	/*
	 * Test if a device without the active DMA flags is not removed upon
	 * the active DMA remove call
	 */
	ut_assertok(device_unbind(dev));
	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);

	/* Probe the device */
	ut_assertok(device_probe(dev));

	/* Test if device is active right now */
	ut_asserteq(true, device_active(dev));

	/* Remove the device via selective remove flag */
	dm_remove_devices_flags(DM_REMOVE_ACTIVE_ALL);

	/* Test if device is still active right now */
	ut_asserteq(true, device_active(dev));

	return 0;
}
DM_TEST(dm_test_remove_active_dma, 0);

/* Test removal of 'vital' devices */
static int dm_test_remove_vital(struct unit_test_state *uts)
{
	struct udevice *normal, *dma, *vital, *dma_vital;

	/* Skip the behaviour in test_post_probe() */
	uts->skip_post_probe = 1;

	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_manual,
					&normal));
	ut_assertnonnull(normal);

	ut_assertok(device_bind_by_name(uts->root, false, &driver_info_act_dma,
					&dma));
	ut_assertnonnull(dma);

	ut_assertok(device_bind_by_name(uts->root, false,
					&driver_info_vital_clk, &vital));
	ut_assertnonnull(vital);

	ut_assertok(device_bind_by_name(uts->root, false,
					&driver_info_act_dma_vital_clk,
					&dma_vital));
	ut_assertnonnull(dma_vital);

	/* Probe the devices */
	ut_assertok(device_probe(normal));
	ut_assertok(device_probe(dma));
	ut_assertok(device_probe(vital));
	ut_assertok(device_probe(dma_vital));

	/* Check that devices are active right now */
	ut_asserteq(true, device_active(normal));
	ut_asserteq(true, device_active(dma));
	ut_asserteq(true, device_active(vital));
	ut_asserteq(true, device_active(dma_vital));

	/* Remove active devices via selective remove flag */
	dm_remove_devices_flags(DM_REMOVE_NON_VITAL | DM_REMOVE_ACTIVE_ALL);

	/*
	 * Check that this only has an effect on the dma device, since two
	 * devices are vital and the third does not have active DMA
	 */
	ut_asserteq(true, device_active(normal));
	ut_asserteq(false, device_active(dma));
	ut_asserteq(true, device_active(vital));
	ut_asserteq(true, device_active(dma_vital));

	/* Remove active devices via selective remove flag */
	ut_assertok(device_probe(dma));
	dm_remove_devices_flags(DM_REMOVE_ACTIVE_ALL);

	/* This should have affected both active-dma devices */
	ut_asserteq(true, device_active(normal));
	ut_asserteq(false, device_active(dma));
	ut_asserteq(true, device_active(vital));
	ut_asserteq(false, device_active(dma_vital));

	/* Remove non-vital devices */
	ut_assertok(device_probe(dma));
	ut_assertok(device_probe(dma_vital));
	dm_remove_devices_flags(DM_REMOVE_NON_VITAL);

	/* This should have affected only non-vital devices */
	ut_asserteq(false, device_active(normal));
	ut_asserteq(false, device_active(dma));
	ut_asserteq(true, device_active(vital));
	ut_asserteq(true, device_active(dma_vital));

	/* Remove vital devices via normal remove flag */
	ut_assertok(device_probe(normal));
	ut_assertok(device_probe(dma));
	dm_remove_devices_flags(DM_REMOVE_NORMAL);

	/* Check that all devices are inactive right now */
	ut_asserteq(false, device_active(normal));
	ut_asserteq(false, device_active(dma));
	ut_asserteq(false, device_active(vital));
	ut_asserteq(false, device_active(dma_vital));

	return 0;
}
DM_TEST(dm_test_remove_vital, 0);

static int dm_test_uclass_before_ready(struct unit_test_state *uts)
{
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));

	gd->dm_root = NULL;
	gd->dm_root_f = NULL;
	memset(&gd->uclass_root, '\0', sizeof(gd->uclass_root));

	ut_asserteq_ptr(NULL, uclass_find(UCLASS_TEST));
	ut_asserteq(-EDEADLK, uclass_get(UCLASS_TEST, &uc));

	return 0;
}
DM_TEST(dm_test_uclass_before_ready, 0);

static int dm_test_uclass_devices_find(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	for (ret = uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_find_next_device(&dev)) {
		ut_assert(!ret);
		ut_assertnonnull(dev);
	}

	ut_assertok(uclass_find_first_device(UCLASS_TEST_DUMMY, &dev));
	ut_assertnull(dev);

	return 0;
}
DM_TEST(dm_test_uclass_devices_find, UT_TESTF_SCAN_PDATA);

static int dm_test_uclass_devices_find_by_name(struct unit_test_state *uts)
{
	struct udevice *finddev;
	struct udevice *testdev;
	int findret, ret;

	/*
	 * For each test device found in fdt like: "a-test", "b-test", etc.,
	 * use its name and try to find it by uclass_find_device_by_name().
	 * Then, on success check if:
	 * - current 'testdev' name is equal to the returned 'finddev' name
	 * - current 'testdev' pointer is equal to the returned 'finddev'
	 *
	 * We assume that, each uclass's device name is unique, so if not, then
	 * this will fail on checking condition: testdev == finddev, since the
	 * uclass_find_device_by_name(), returns the first device by given name.
	*/
	for (ret = uclass_find_first_device(UCLASS_TEST_FDT, &testdev);
	     testdev;
	     ret = uclass_find_next_device(&testdev)) {
		ut_assertok(ret);
		ut_assertnonnull(testdev);

		findret = uclass_find_device_by_name(UCLASS_TEST_FDT,
						     testdev->name,
						     &finddev);

		ut_assertok(findret);
		ut_assert(testdev);
		ut_asserteq_str(testdev->name, finddev->name);
		ut_asserteq_ptr(testdev, finddev);
	}

	return 0;
}
DM_TEST(dm_test_uclass_devices_find_by_name, UT_TESTF_SCAN_FDT);

static int dm_test_uclass_devices_get(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	for (ret = uclass_first_device_check(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_next_device_check(&dev)) {
		ut_assert(!ret);
		ut_assert(device_active(dev));
	}

	return 0;
}
DM_TEST(dm_test_uclass_devices_get, UT_TESTF_SCAN_PDATA);

static int dm_test_uclass_devices_get_by_name(struct unit_test_state *uts)
{
	struct udevice *finddev;
	struct udevice *testdev;
	int ret, findret;

	/*
	 * For each test device found in fdt like: "a-test", "b-test", etc.,
	 * use its name and try to get it by uclass_get_device_by_name().
	 * On success check if:
	 * - returned finddev' is active
	 * - current 'testdev' name is equal to the returned 'finddev' name
	 * - current 'testdev' pointer is equal to the returned 'finddev'
	 *
	 * We asserts that the 'testdev' is active on each loop entry, so we
	 * could be sure that the 'finddev' is activated too, but for sure
	 * we check it again.
	 *
	 * We assume that, each uclass's device name is unique, so if not, then
	 * this will fail on checking condition: testdev == finddev, since the
	 * uclass_get_device_by_name(), returns the first device by given name.
	*/
	for (ret = uclass_first_device_check(UCLASS_TEST_FDT, &testdev);
	     testdev;
	     ret = uclass_next_device_check(&testdev)) {
		ut_assertok(ret);
		ut_assert(device_active(testdev));

		findret = uclass_get_device_by_name(UCLASS_TEST_FDT,
						    testdev->name,
						    &finddev);

		ut_assertok(findret);
		ut_assert(finddev);
		ut_assert(device_active(finddev));
		ut_asserteq_str(testdev->name, finddev->name);
		ut_asserteq_ptr(testdev, finddev);
	}

	return 0;
}
DM_TEST(dm_test_uclass_devices_get_by_name, UT_TESTF_SCAN_FDT);

static int dm_test_device_get_uclass_id(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST, 0, &dev));
	ut_asserteq(UCLASS_TEST, device_get_uclass_id(dev));

	return 0;
}
DM_TEST(dm_test_device_get_uclass_id, UT_TESTF_SCAN_PDATA);

static int dm_test_uclass_names(struct unit_test_state *uts)
{
	ut_asserteq_str("test", uclass_get_name(UCLASS_TEST));
	ut_asserteq(UCLASS_TEST, uclass_get_by_name("test"));

	ut_asserteq(UCLASS_SPI, uclass_get_by_name("spi"));

	return 0;
}
DM_TEST(dm_test_uclass_names, UT_TESTF_SCAN_PDATA);

static int dm_test_inactive_child(struct unit_test_state *uts)
{
	struct udevice *parent, *dev1, *dev2;

	/* Skip the behaviour in test_post_probe() */
	uts->skip_post_probe = 1;

	ut_assertok(uclass_first_device_err(UCLASS_TEST, &parent));

	/*
	 * Create a child but do not activate it. Calling the function again
	 * should return the same child.
	 */
	ut_asserteq(-ENODEV, device_find_first_inactive_child(parent,
							UCLASS_TEST, &dev1));
	ut_assertok(device_bind(parent, DM_DRIVER_GET(test_drv),
				"test_child", 0, ofnode_null(), &dev1));

	ut_assertok(device_find_first_inactive_child(parent, UCLASS_TEST,
						     &dev2));
	ut_asserteq_ptr(dev1, dev2);

	ut_assertok(device_probe(dev1));
	ut_asserteq(-ENODEV, device_find_first_inactive_child(parent,
							UCLASS_TEST, &dev2));

	return 0;
}
DM_TEST(dm_test_inactive_child, UT_TESTF_SCAN_PDATA);

/* Make sure all bound devices have a sequence number */
static int dm_test_all_have_seq(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct uclass *uc;

	list_for_each_entry(uc, gd->uclass_root, sibling_node) {
		list_for_each_entry(dev, &uc->dev_head, uclass_node) {
			if (dev->seq_ == -1)
				printf("Device '%s' has no seq (%d)\n",
				       dev->name, dev->seq_);
			ut_assert(dev->seq_ != -1);
		}
	}

	return 0;
}
DM_TEST(dm_test_all_have_seq, UT_TESTF_SCAN_PDATA);

#if CONFIG_IS_ENABLED(DM_DMA)
static int dm_test_dma_offset(struct unit_test_state *uts)
{
       struct udevice *dev;
       ofnode node;

       /* Make sure the bus's dma-ranges aren't taken into account here */
       node = ofnode_path("/mmio-bus@0");
       ut_assert(ofnode_valid(node));
       ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_BUS, node, &dev));
       ut_asserteq_64(0, dev->dma_offset);

       /* Device behind a bus with dma-ranges */
       node = ofnode_path("/mmio-bus@0/subnode@0");
       ut_assert(ofnode_valid(node));
       ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_FDT, node, &dev));
       ut_asserteq_64(-0x10000000ULL, dev->dma_offset);

       /* This one has no dma-ranges */
       node = ofnode_path("/mmio-bus@1");
       ut_assert(ofnode_valid(node));
       ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_BUS, node, &dev));
       node = ofnode_path("/mmio-bus@1/subnode@0");
       ut_assert(ofnode_valid(node));
       ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_FDT, node, &dev));
       ut_asserteq_64(0, dev->dma_offset);

       return 0;
}
DM_TEST(dm_test_dma_offset, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
#endif

/* Test dm_get_stats() */
static int dm_test_get_stats(struct unit_test_state *uts)
{
	int dev_count, uc_count;

	dm_get_stats(&dev_count, &uc_count);
	ut_assert(dev_count > 50);
	ut_assert(uc_count > 30);

	return 0;
}
DM_TEST(dm_test_get_stats, UT_TESTF_SCAN_FDT);

/* Test uclass_find_device_by_name() */
static int dm_test_uclass_find_device(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_find_device_by_name(UCLASS_I2C, "i2c@0", &dev));
	ut_asserteq(-ENODEV,
		    uclass_find_device_by_name(UCLASS_I2C, "i2c@0x", &dev));
	ut_assertok(uclass_find_device_by_namelen(UCLASS_I2C, "i2c@0x", 5,
						  &dev));

	return 0;
}
DM_TEST(dm_test_uclass_find_device, UT_TESTF_SCAN_FDT);

/* Test getting information about tags attached to devices */
static int dm_test_dev_get_attach(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertnonnull(dev_get_attach_ptr(dev, DM_TAG_PLAT));
	ut_assertnonnull(dev_get_attach_ptr(dev, DM_TAG_PRIV));
	ut_assertnull(dev_get_attach_ptr(dev, DM_TAG_UC_PRIV));
	ut_assertnull(dev_get_attach_ptr(dev, DM_TAG_UC_PLAT));
	ut_assertnull(dev_get_attach_ptr(dev, DM_TAG_PARENT_PLAT));
	ut_assertnull(dev_get_attach_ptr(dev, DM_TAG_PARENT_PRIV));

	ut_asserteq(sizeof(struct dm_test_pdata),
		    dev_get_attach_size(dev, DM_TAG_PLAT));
	ut_asserteq(sizeof(struct dm_test_priv),
		    dev_get_attach_size(dev, DM_TAG_PRIV));
	ut_asserteq(0, dev_get_attach_size(dev, DM_TAG_UC_PRIV));
	ut_asserteq(0, dev_get_attach_size(dev, DM_TAG_UC_PLAT));
	ut_asserteq(0, dev_get_attach_size(dev, DM_TAG_PARENT_PLAT));
	ut_asserteq(0, dev_get_attach_size(dev, DM_TAG_PARENT_PRIV));

	return 0;
}
DM_TEST(dm_test_dev_get_attach, UT_TESTF_SCAN_FDT);

/* Test getting information about tags attached to bus devices */
static int dm_test_dev_get_attach_bus(struct unit_test_state *uts)
{
	struct udevice *dev, *child;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_BUS, &dev));
	ut_asserteq_str("some-bus", dev->name);

	ut_assertnonnull(dev_get_attach_ptr(dev, DM_TAG_PLAT));
	ut_assertnonnull(dev_get_attach_ptr(dev, DM_TAG_PRIV));
	ut_assertnonnull(dev_get_attach_ptr(dev, DM_TAG_UC_PRIV));
	ut_assertnonnull(dev_get_attach_ptr(dev, DM_TAG_UC_PLAT));
	ut_assertnull(dev_get_attach_ptr(dev, DM_TAG_PARENT_PLAT));
	ut_assertnull(dev_get_attach_ptr(dev, DM_TAG_PARENT_PRIV));

	ut_asserteq(sizeof(struct dm_test_pdata),
		    dev_get_attach_size(dev, DM_TAG_PLAT));
	ut_asserteq(sizeof(struct dm_test_priv),
		    dev_get_attach_size(dev, DM_TAG_PRIV));
	ut_asserteq(sizeof(struct dm_test_uclass_priv),
		    dev_get_attach_size(dev, DM_TAG_UC_PRIV));
	ut_asserteq(sizeof(struct dm_test_uclass_plat),
		    dev_get_attach_size(dev, DM_TAG_UC_PLAT));
	ut_asserteq(0, dev_get_attach_size(dev, DM_TAG_PARENT_PLAT));
	ut_asserteq(0, dev_get_attach_size(dev, DM_TAG_PARENT_PRIV));

	/* Now try the child of the bus */
	ut_assertok(device_first_child_err(dev, &child));
	ut_asserteq_str("c-test@5", child->name);

	ut_assertnonnull(dev_get_attach_ptr(child, DM_TAG_PLAT));
	ut_assertnonnull(dev_get_attach_ptr(child, DM_TAG_PRIV));
	ut_assertnull(dev_get_attach_ptr(child, DM_TAG_UC_PRIV));
	ut_assertnull(dev_get_attach_ptr(child, DM_TAG_UC_PLAT));
	ut_assertnonnull(dev_get_attach_ptr(child, DM_TAG_PARENT_PLAT));
	ut_assertnonnull(dev_get_attach_ptr(child, DM_TAG_PARENT_PRIV));

	ut_asserteq(sizeof(struct dm_test_pdata),
		    dev_get_attach_size(child, DM_TAG_PLAT));
	ut_asserteq(sizeof(struct dm_test_priv),
		    dev_get_attach_size(child, DM_TAG_PRIV));
	ut_asserteq(0, dev_get_attach_size(child, DM_TAG_UC_PRIV));
	ut_asserteq(0, dev_get_attach_size(child, DM_TAG_UC_PLAT));
	ut_asserteq(sizeof(struct dm_test_parent_plat),
		    dev_get_attach_size(child, DM_TAG_PARENT_PLAT));
	ut_asserteq(sizeof(struct dm_test_parent_data),
		    dev_get_attach_size(child, DM_TAG_PARENT_PRIV));

	return 0;
}
DM_TEST(dm_test_dev_get_attach_bus, UT_TESTF_SCAN_FDT);

/* Test getting information about tags attached to bus devices */
static int dm_test_dev_get_mem(struct unit_test_state *uts)
{
	struct dm_stats stats;

	dm_get_mem(&stats);

	return 0;
}
DM_TEST(dm_test_dev_get_mem, UT_TESTF_SCAN_FDT);
