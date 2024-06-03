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
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
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

U_BOOT_DEVICE(dm_test_info1) = {
	.name = "test_drv",
	.platdata = &test_pdata[0],
};

U_BOOT_DEVICE(dm_test_info2) = {
	.name = "test_drv",
	.platdata = &test_pdata[1],
};

U_BOOT_DEVICE(dm_test_info3) = {
	.name = "test_drv",
	.platdata = &test_pdata[2],
};

static struct driver_info driver_info_manual = {
	.name = "test_manual_drv",
	.platdata = &test_pdata_manual,
};

static struct driver_info driver_info_pre_reloc = {
	.name = "test_pre_reloc_drv",
	.platdata = &test_pdata_pre_reloc,
};

static struct driver_info driver_info_act_dma = {
	.name = "test_act_dma_drv",
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

/* Test that binding with platdata occurs correctly */
static int dm_test_autobind(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
	struct udevice *dev;

	/*
	 * We should have a single class (UCLASS_ROOT) and a single root
	 * device with no children.
	 */
	ut_assert(dms->root);
	ut_asserteq(1, list_count_items(&gd->uclass_root));
	ut_asserteq(0, list_count_items(&gd->dm_root->child_head));
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_POST_BIND]);

	ut_assertok(dm_scan_platdata(false));

	/* We should have our test class now at least, plus more children */
	ut_assert(1 < list_count_items(&gd->uclass_root));
	ut_assert(0 < list_count_items(&gd->dm_root->child_head));

	/* Our 3 dm_test_infox children should be bound to the test uclass */
	ut_asserteq(3, dm_testdrv_op_count[DM_TEST_OP_POST_BIND]);

	/* No devices should be probed */
	list_for_each_entry(dev, &gd->dm_root->child_head, sibling_node)
		ut_assert(!(dev->flags & DM_FLAG_ACTIVATED));

	/* Our test driver should have been bound 3 times */
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_BIND] == 3);

	return 0;
}
DM_TEST(dm_test_autobind, 0);

/* Test that binding with uclass platdata allocation occurs correctly */
static int dm_test_autobind_uclass_pdata_alloc(struct unit_test_state *uts)
{
	struct dm_test_perdev_uc_pdata *uc_pdata;
	struct udevice *dev;
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_assert(uc);

	/**
	 * Test if test uclass driver requires allocation for the uclass
	 * platform data and then check the dev->uclass_platdata pointer.
	 */
	ut_assert(uc->uc_drv->per_device_platdata_auto_alloc_size);

	for (uclass_find_first_device(UCLASS_TEST, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		ut_assert(dev);

		uc_pdata = dev_get_uclass_platdata(dev);
		ut_assert(uc_pdata);
	}

	return 0;
}
DM_TEST(dm_test_autobind_uclass_pdata_alloc, DM_TESTF_SCAN_PDATA);

/* Test that binding with uclass platdata setting occurs correctly */
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
		ut_assert(dev);

		uc_pdata = dev_get_uclass_platdata(dev);
		ut_assert(uc_pdata);
		ut_assert(uc_pdata->intval1 == TEST_UC_PDATA_INTVAL1);
		ut_assert(uc_pdata->intval2 == TEST_UC_PDATA_INTVAL2);
		ut_assert(uc_pdata->intval3 == TEST_UC_PDATA_INTVAL3);
	}

	return 0;
}
DM_TEST(dm_test_autobind_uclass_pdata_valid, DM_TESTF_SCAN_PDATA);

/* Test that autoprobe finds all the expected devices */
static int dm_test_autoprobe(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
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
	ut_assert(dms->root->flags & DM_FLAG_ACTIVATED);

	/*
	 * We should be able to find the three test devices, and they should
	 * all be activated as they are used (lazy activation, required by
	 * U-Boot)
	 */
	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assertf(!(dev->flags & DM_FLAG_ACTIVATED),
			   "Driver %d/%s already activated", i, dev->name);

		/* This should activate it */
		ut_assertok(uclass_get_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assert(dev->flags & DM_FLAG_ACTIVATED);

		/* Activating a device should activate the root device */
		if (!i)
			ut_assert(dms->root->flags & DM_FLAG_ACTIVATED);
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

		pdata = dev->platdata;
		expected_base_add += pdata->ping_add;
	}

	return 0;
}
DM_TEST(dm_test_autoprobe, DM_TESTF_SCAN_PDATA);

/* Check that we see the correct platdata in each device */
static int dm_test_platdata(struct unit_test_state *uts)
{
	const struct dm_test_pdata *pdata;
	struct udevice *dev;
	int i;

	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		pdata = dev->platdata;
		ut_assert(pdata->ping_add == test_pdata[i].ping_add);
	}

	return 0;
}
DM_TEST(dm_test_platdata, DM_TESTF_SCAN_PDATA);

/* Test that we can bind, probe, remove, unbind a driver */
static int dm_test_lifecycle(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
	int op_count[DM_TEST_OP_COUNT];
	struct udevice *dev, *test_dev;
	int pingret;
	int ret;

	memcpy(op_count, dm_testdrv_op_count, sizeof(op_count));

	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_BIND]
			== op_count[DM_TEST_OP_BIND] + 1);
	ut_assert(!dev->priv);

	/* Probe the device - it should fail allocating private data */
	dms->force_fail_alloc = 1;
	ret = device_probe(dev);
	ut_assert(ret == -ENOMEM);
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_PROBE]
			== op_count[DM_TEST_OP_PROBE] + 1);
	ut_assert(!dev->priv);

	/* Try again without the alloc failure */
	dms->force_fail_alloc = 0;
	ut_assertok(device_probe(dev));
	ut_assert(dm_testdrv_op_count[DM_TEST_OP_PROBE]
			== op_count[DM_TEST_OP_PROBE] + 2);
	ut_assert(dev->priv);

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

	return 0;
}
DM_TEST(dm_test_lifecycle, DM_TESTF_SCAN_PDATA | DM_TESTF_PROBE_TEST);

/* Test that we can bind/unbind and the lists update correctly */
static int dm_test_ordering(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
	struct udevice *dev, *dev_penultimate, *dev_last, *test_dev;
	int pingret;

	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev));
	ut_assert(dev);

	/* Bind two new devices (numbers 4 and 5) */
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
					&dev_penultimate));
	ut_assert(dev_penultimate);
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
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
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
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
DM_TEST(dm_test_ordering, DM_TESTF_SCAN_PDATA);

/* Check that we can perform operations on a device (do a ping) */
int dm_check_operations(struct unit_test_state *uts, struct udevice *dev,
			uint32_t base, struct dm_test_priv *priv)
{
	int expected;
	int pingret;

	/* Getting the child device should allocate platdata / priv */
	ut_assertok(testfdt_ping(dev, 10, &pingret));
	ut_assert(dev->priv);
	ut_assert(dev->platdata);

	expected = 10 + base;
	ut_asserteq(expected, pingret);

	/* Do another ping */
	ut_assertok(testfdt_ping(dev, 20, &pingret));
	expected = 20 + base;
	ut_asserteq(expected, pingret);

	/* Now check the ping_total */
	priv = dev->priv;
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
		 * should be. We don't use the platdata because we want
		 * to test the code that sets that up (testfdt_drv_probe()).
		 */
		base = test_pdata[i].ping_add;
		debug("dev=%d, base=%d\n", i, base);

		ut_assert(!dm_check_operations(uts, dev, base, dev->priv));
	}

	return 0;
}
DM_TEST(dm_test_operations, DM_TESTF_SCAN_PDATA);

/* Remove all drivers and check that things work */
static int dm_test_remove(struct unit_test_state *uts)
{
	struct udevice *dev;
	int i;

	for (i = 0; i < 3; i++) {
		ut_assertok(uclass_find_device(UCLASS_TEST, i, &dev));
		ut_assert(dev);
		ut_assertf(dev->flags & DM_FLAG_ACTIVATED,
			   "Driver %d/%s not activated", i, dev->name);
		ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
		ut_assertf(!(dev->flags & DM_FLAG_ACTIVATED),
			   "Driver %d/%s should have deactivated", i,
			   dev->name);
		ut_assert(!dev->priv);
	}

	return 0;
}
DM_TEST(dm_test_remove, DM_TESTF_SCAN_PDATA | DM_TESTF_PROBE_TEST);

/* Remove and recreate everything, check for memory leaks */
static int dm_test_leak(struct unit_test_state *uts)
{
	int i;

	for (i = 0; i < 2; i++) {
		struct udevice *dev;
		int ret;
		int id;

		dm_leak_check_start(uts);

		ut_assertok(dm_scan_platdata(false));
		ut_assertok(dm_scan_fdt(gd->fdt_blob, false));

		/* Scanning the uclass is enough to probe all the devices */
		for (id = UCLASS_ROOT; id < UCLASS_COUNT; id++) {
			for (ret = uclass_first_device(UCLASS_TEST, &dev);
			     dev;
			     ret = uclass_next_device(&dev))
				;
			ut_assertok(ret);
		}

		ut_assertok(dm_leak_check_end(uts));
	}

	return 0;
}
DM_TEST(dm_test_leak, 0);

/* Test uclass init/destroy methods */
static int dm_test_uclass(struct unit_test_state *uts)
{
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(0, dm_testdrv_op_count[DM_TEST_OP_DESTROY]);
	ut_assert(uc->priv);

	ut_assertok(uclass_destroy(uc));
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_INIT]);
	ut_asserteq(1, dm_testdrv_op_count[DM_TEST_OP_DESTROY]);

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
 * @return 0 if OK, -ve on error
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
		dev->platdata = pdata;
		if (child)
			child[i] = dev;
	}

	return 0;
}

#define NODE_COUNT	10

static int dm_test_children(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
	struct udevice *top[NODE_COUNT];
	struct udevice *child[NODE_COUNT];
	struct udevice *grandchild[NODE_COUNT];
	struct udevice *dev;
	int total;
	int ret;
	int i;

	/* We don't care about the numbering for this test */
	dms->skip_post_probe = 1;

	ut_assert(NODE_COUNT > 5);

	/* First create 10 top-level children */
	ut_assertok(create_children(uts, dms->root, NODE_COUNT, 0, top));

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
	for (ret = uclass_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_next_device(&dev))
		;
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

/* Test that pre-relocation devices work as expected */
static int dm_test_pre_reloc(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
	struct udevice *dev;

	/* The normal driver should refuse to bind before relocation */
	ut_asserteq(-EPERM, device_bind_by_name(dms->root, true,
						&driver_info_manual, &dev));

	/* But this one is marked pre-reloc */
	ut_assertok(device_bind_by_name(dms->root, true,
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
	struct dm_test_state *dms = uts->priv;
	struct udevice *dev;

	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_act_dma,
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
	ut_assertok(device_bind_by_name(dms->root, false, &driver_info_manual,
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

static int dm_test_uclass_before_ready(struct unit_test_state *uts)
{
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST, &uc));

	gd->dm_root = NULL;
	gd->dm_root_f = NULL;
	memset(&gd->uclass_root, '\0', sizeof(gd->uclass_root));

	ut_asserteq_ptr(NULL, uclass_find(UCLASS_TEST));

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
		ut_assert(dev);
	}

	ret = uclass_find_first_device(UCLASS_TEST_DUMMY, &dev);
	ut_assert(ret == -ENODEV);
	ut_assert(!dev);

	return 0;
}
DM_TEST(dm_test_uclass_devices_find, DM_TESTF_SCAN_PDATA);

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
		ut_assert(testdev);

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
DM_TEST(dm_test_uclass_devices_find_by_name, DM_TESTF_SCAN_FDT);

static int dm_test_uclass_devices_get(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	for (ret = uclass_first_device(UCLASS_TEST, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		ut_assert(!ret);
		ut_assert(dev);
		ut_assert(device_active(dev));
	}

	return 0;
}
DM_TEST(dm_test_uclass_devices_get, DM_TESTF_SCAN_PDATA);

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
	for (ret = uclass_first_device(UCLASS_TEST_FDT, &testdev);
	     testdev;
	     ret = uclass_next_device(&testdev)) {
		ut_assertok(ret);
		ut_assert(testdev);
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
DM_TEST(dm_test_uclass_devices_get_by_name, DM_TESTF_SCAN_FDT);

static int dm_test_device_get_uclass_id(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST, 0, &dev));
	ut_asserteq(UCLASS_TEST, device_get_uclass_id(dev));

	return 0;
}
DM_TEST(dm_test_device_get_uclass_id, DM_TESTF_SCAN_PDATA);

static int dm_test_uclass_names(struct unit_test_state *uts)
{
	ut_asserteq_str("test", uclass_get_name(UCLASS_TEST));
	ut_asserteq(UCLASS_TEST, uclass_get_by_name("test"));

	return 0;
}
DM_TEST(dm_test_uclass_names, DM_TESTF_SCAN_PDATA);

static int dm_test_inactive_child(struct unit_test_state *uts)
{
	struct dm_test_state *dms = uts->priv;
	struct udevice *parent, *dev1, *dev2;

	/* Skip the behaviour in test_post_probe() */
	dms->skip_post_probe = 1;

	ut_assertok(uclass_first_device_err(UCLASS_TEST, &parent));

	/*
	 * Create a child but do not activate it. Calling the function again
	 * should return the same child.
	 */
	ut_asserteq(-ENODEV, device_find_first_inactive_child(parent,
							UCLASS_TEST, &dev1));
	ut_assertok(device_bind_ofnode(parent, DM_GET_DRIVER(test_drv),
				       "test_child", 0, ofnode_null(), &dev1));

	ut_assertok(device_find_first_inactive_child(parent, UCLASS_TEST,
						     &dev2));
	ut_asserteq_ptr(dev1, dev2);

	ut_assertok(device_probe(dev1));
	ut_asserteq(-ENODEV, device_find_first_inactive_child(parent,
							UCLASS_TEST, &dev2));

	return 0;
}
DM_TEST(dm_test_inactive_child, DM_TESTF_SCAN_PDATA);
