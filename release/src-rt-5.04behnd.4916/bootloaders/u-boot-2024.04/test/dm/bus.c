// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#ifdef CONFIG_SANDBOX
#include <log.h>
#include <os.h>
#endif
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test that we can probe for children */
static int dm_test_bus_children(struct unit_test_state *uts)
{
	int num_devices = 9;
	struct udevice *bus;
	struct uclass *uc;

	ut_assertok(uclass_get(UCLASS_TEST_FDT, &uc));
	ut_asserteq(num_devices, list_count_items(&uc->dev_head));

	/* Probe the bus, which should yield 3 more devices */
	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	num_devices += 3;

	ut_assertok(uclass_get(UCLASS_TEST_FDT, &uc));
	ut_asserteq(num_devices, list_count_items(&uc->dev_head));

	ut_assert(!dm_check_devices(uts, num_devices));

	return 0;
}
DM_TEST(dm_test_bus_children, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test our functions for accessing children */
static int dm_test_bus_children_funcs(struct unit_test_state *uts)
{
	const void *blob = gd->fdt_blob;
	struct udevice *bus, *dev;
	int node;

	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));

	/* device_get_child() */
	ut_assertok(device_get_child(bus, 0, &dev));
	ut_asserteq(-ENODEV, device_get_child(bus, 4, &dev));
	ut_assertok(device_get_child_by_seq(bus, 5, &dev));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);
	ut_asserteq_str("c-test@5", dev->name);

	/* Device with sequence number 0 should be accessible */
	ut_asserteq(-ENODEV, device_find_child_by_seq(bus, -1, &dev));
	ut_assertok(device_find_child_by_seq(bus, 0, &dev));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));
	ut_asserteq(0, device_find_child_by_seq(bus, 0, &dev));
	ut_assertok(device_get_child_by_seq(bus, 0, &dev));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);
	ut_asserteq(0, device_find_child_by_seq(bus, 0, &dev));

	/* There is no device with sequence number 2 */
	ut_asserteq(-ENODEV, device_find_child_by_seq(bus, 2, &dev));
	ut_asserteq(-ENODEV, device_find_child_by_seq(bus, 2, &dev));
	ut_asserteq(-ENODEV, device_get_child_by_seq(bus, 2, &dev));

	/* Looking for something that is not a child */
	node = fdt_path_offset(blob, "/junk");
	ut_asserteq(-ENODEV, device_find_child_by_of_offset(bus, node, &dev));
	node = fdt_path_offset(blob, "/d-test");
	ut_asserteq(-ENODEV, device_find_child_by_of_offset(bus, node, &dev));

	return 0;
}
DM_TEST(dm_test_bus_children_funcs, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_bus_children_of_offset(struct unit_test_state *uts)
{
	const void *blob = gd->fdt_blob;
	struct udevice *bus, *dev;
	int node;

	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	ut_assertnonnull(bus);

	/* Find a valid child */
	node = fdt_path_offset(blob, "/some-bus/c-test@1");
	ut_assert(node > 0);
	ut_assertok(device_find_child_by_of_offset(bus, node, &dev));
	ut_assertnonnull(dev);
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));
	ut_assertok(device_get_child_by_of_offset(bus, node, &dev));
	ut_assertnonnull(dev);
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);

	return 0;
}
DM_TEST(dm_test_bus_children_of_offset,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

/* Test that we can iterate through children */
static int dm_test_bus_children_iterators(struct unit_test_state *uts)
{
	struct udevice *bus, *dev, *child;

	/* Walk through the children one by one */
	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	ut_assertok(device_find_first_child(bus, &dev));
	ut_asserteq_str("c-test@5", dev->name);
	ut_assertok(device_find_next_child(&dev));
	ut_asserteq_str("c-test@0", dev->name);
	ut_assertok(device_find_next_child(&dev));
	ut_asserteq_str("c-test@1", dev->name);
	ut_assertok(device_find_next_child(&dev));
	ut_asserteq_ptr(dev, NULL);

	/* Move to the next child without using device_find_first_child() */
	ut_assertok(device_find_child_by_seq(bus, 5, &dev));
	ut_asserteq_str("c-test@5", dev->name);
	ut_assertok(device_find_next_child(&dev));
	ut_asserteq_str("c-test@0", dev->name);

	/* Try a device with no children */
	ut_assertok(device_find_first_child(dev, &child));
	ut_asserteq_ptr(child, NULL);

	return 0;
}
DM_TEST(dm_test_bus_children_iterators,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that the bus can store data about each child */
static int test_bus_parent_data(struct unit_test_state *uts)
{
	struct dm_test_parent_data *parent_data;
	struct udevice *bus, *dev;
	struct uclass *uc;
	int value;

	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));

	/* Check that parent data is allocated */
	ut_assertok(device_find_child_by_seq(bus, 0, &dev));
	ut_asserteq_ptr(NULL, dev_get_parent_priv(dev));
	ut_assertok(device_get_child_by_seq(bus, 0, &dev));
	parent_data = dev_get_parent_priv(dev);
	ut_assert(NULL != parent_data);

	/* Check that it starts at 0 and goes away when device is removed */
	parent_data->sum += 5;
	ut_asserteq(5, parent_data->sum);
	device_remove(dev, DM_REMOVE_NORMAL);
	ut_asserteq_ptr(NULL, dev_get_parent_priv(dev));

	/* Check that we can do this twice */
	ut_assertok(device_get_child_by_seq(bus, 0, &dev));
	parent_data = dev_get_parent_priv(dev);
	ut_assert(NULL != parent_data);
	parent_data->sum += 5;
	ut_asserteq(5, parent_data->sum);

	/* Add parent data to all children */
	ut_assertok(uclass_get(UCLASS_TEST_FDT, &uc));
	value = 5;
	uclass_foreach_dev(dev, uc) {
		/* Ignore these if they are not on this bus */
		if (dev->parent != bus) {
			ut_asserteq_ptr(NULL, dev_get_parent_priv(dev));
			continue;
		}
		ut_assertok(device_probe(dev));
		parent_data = dev_get_parent_priv(dev);

		parent_data->sum = value;
		value += 5;
	}

	/* Check it is still there */
	value = 5;
	uclass_foreach_dev(dev, uc) {
		/* Ignore these if they are not on this bus */
		if (dev->parent != bus)
			continue;
		parent_data = dev_get_parent_priv(dev);

		ut_asserteq(value, parent_data->sum);
		value += 5;
	}

	return 0;
}
/* Test that the bus can store data about each child */
static int dm_test_bus_parent_data(struct unit_test_state *uts)
{
	return test_bus_parent_data(uts);
}
DM_TEST(dm_test_bus_parent_data, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* As above but the size is controlled by the uclass */
static int dm_test_bus_parent_data_uclass(struct unit_test_state *uts)
{
	struct driver *drv;
	struct udevice *bus;
	int size;
	int ret;

	/* Set the driver size to 0 so that the uclass size is used */
	ut_assertok(uclass_find_device(UCLASS_TEST_BUS, 0, &bus));
	drv = (struct driver *)bus->driver;
	size = drv->per_child_auto;

#ifdef CONFIG_SANDBOX
	os_mprotect_allow(bus->uclass->uc_drv, sizeof(*bus->uclass->uc_drv));
	os_mprotect_allow(drv, sizeof(*drv));
#endif
	bus->uclass->uc_drv->per_child_auto = size;
	drv->per_child_auto = 0;
	ret = test_bus_parent_data(uts);
	if (ret)
		return ret;
	bus->uclass->uc_drv->per_child_auto = 0;
	drv->per_child_auto = size;

	return 0;
}
DM_TEST(dm_test_bus_parent_data_uclass,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that the bus ops are called when a child is probed/removed */
static int dm_test_bus_parent_ops(struct unit_test_state *uts)
{
	struct dm_test_parent_data *parent_data;
	struct udevice *bus, *dev;
	struct uclass *uc;

	testbus_get_clear_removed();
	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	ut_assertok(uclass_get(UCLASS_TEST_FDT, &uc));

	uclass_foreach_dev(dev, uc) {
		/* Ignore these if they are not on this bus */
		if (dev->parent != bus)
			continue;
		ut_asserteq_ptr(NULL, dev_get_parent_priv(dev));

		ut_assertok(device_probe(dev));
		parent_data = dev_get_parent_priv(dev);
		ut_asserteq(TEST_FLAG_CHILD_PROBED, parent_data->flag);
	}

	uclass_foreach_dev(dev, uc) {
		/* Ignore these if they are not on this bus */
		if (dev->parent != bus)
			continue;
		parent_data = dev_get_parent_priv(dev);
		ut_asserteq(TEST_FLAG_CHILD_PROBED, parent_data->flag);
		ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));
		ut_asserteq_ptr(NULL, dev_get_parent_priv(dev));
		ut_asserteq_ptr(testbus_get_clear_removed(), dev);
	}

	return 0;
}
DM_TEST(dm_test_bus_parent_ops, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int test_bus_parent_plat(struct unit_test_state *uts)
{
	struct dm_test_parent_plat *plat;
	struct udevice *bus, *dev;

	/* Check that the bus has no children */
	ut_assertok(uclass_find_device(UCLASS_TEST_BUS, 0, &bus));
	device_find_first_child(bus, &dev);
	ut_asserteq_ptr(NULL, dev);

	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));

	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		/* Check that platform data is allocated */
		plat = dev_get_parent_plat(dev);
		ut_assert(plat != NULL);

		/*
		 * Check that it is not affected by the device being
		 * probed/removed
		 */
		plat->count++;
		ut_asserteq(1, plat->count);
		device_probe(dev);
		device_remove(dev, DM_REMOVE_NORMAL);

		ut_asserteq_ptr(plat, dev_get_parent_plat(dev));
		ut_asserteq(1, plat->count);
		ut_assertok(device_probe(dev));
	}
	ut_asserteq(3, device_get_child_count(bus));

	/* Removing the bus should also have no effect (it is still bound) */
	device_remove(bus, DM_REMOVE_NORMAL);
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		/* Check that platform data is allocated */
		plat = dev_get_parent_plat(dev);
		ut_assert(plat != NULL);
		ut_asserteq(1, plat->count);
	}
	ut_asserteq(3, device_get_child_count(bus));

	/* Unbind all the children */
	do {
		device_find_first_child(bus, &dev);
		if (dev)
			device_unbind(dev);
	} while (dev);

	/* Now the child plat should be removed and re-added */
	device_probe(bus);
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		/* Check that platform data is allocated */
		plat = dev_get_parent_plat(dev);
		ut_assert(plat != NULL);
		ut_asserteq(0, plat->count);
	}
	ut_asserteq(3, device_get_child_count(bus));

	return 0;
}

/* Test that the bus can store platform data about each child */
static int dm_test_bus_parent_plat(struct unit_test_state *uts)
{
	return test_bus_parent_plat(uts);
}
DM_TEST(dm_test_bus_parent_plat, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* As above but the size is controlled by the uclass */
static int dm_test_bus_parent_plat_uclass(struct unit_test_state *uts)
{
	struct udevice *bus;
	struct driver *drv;
	int size;
	int ret;

	/* Set the driver size to 0 so that the uclass size is used */
	ut_assertok(uclass_find_device(UCLASS_TEST_BUS, 0, &bus));
	drv = (struct driver *)bus->driver;
	size = drv->per_child_plat_auto;
#ifdef CONFIG_SANDBOX
	os_mprotect_allow(bus->uclass->uc_drv, sizeof(*bus->uclass->uc_drv));
	os_mprotect_allow(drv, sizeof(*drv));
#endif
	bus->uclass->uc_drv->per_child_plat_auto = size;
	drv->per_child_plat_auto = 0;
	ret = test_bus_parent_plat(uts);
	if (ret)
		return ret;
	bus->uclass->uc_drv->per_child_plat_auto = 0;
	drv->per_child_plat_auto = size;

	return 0;
}
DM_TEST(dm_test_bus_parent_plat_uclass,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that the child post_bind method is called */
static int dm_test_bus_child_post_bind(struct unit_test_state *uts)
{
	struct dm_test_parent_plat *plat;
	struct udevice *bus, *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		/* Check that platform data is allocated */
		plat = dev_get_parent_plat(dev);
		ut_assert(plat != NULL);
		ut_asserteq(1, plat->bind_flag);
	}
	ut_asserteq(3, device_get_child_count(bus));

	return 0;
}
DM_TEST(dm_test_bus_child_post_bind, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that the child post_bind method is called */
static int dm_test_bus_child_post_bind_uclass(struct unit_test_state *uts)
{
	struct dm_test_parent_plat *plat;
	struct udevice *bus, *dev;

	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		/* Check that platform data is allocated */
		plat = dev_get_parent_plat(dev);
		ut_assert(plat != NULL);
		ut_asserteq(2, plat->uclass_bind_flag);
	}
	ut_asserteq(3, device_get_child_count(bus));

	return 0;
}
DM_TEST(dm_test_bus_child_post_bind_uclass,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/*
 * Test that the bus' uclass' child_pre_probe() is called before the
 * device's probe() method
 */
static int dm_test_bus_child_pre_probe_uclass(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;

	/*
	 * See testfdt_drv_probe() which effectively checks that the uclass
	 * flag is set before that method is called
	 */
	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		struct dm_test_priv *priv = dev_get_priv(dev);

		/* Check that things happened in the right order */
		ut_asserteq_ptr(NULL, priv);
		ut_assertok(device_probe(dev));

		priv = dev_get_priv(dev);
		ut_assert(priv != NULL);
		ut_asserteq(1, priv->uclass_flag);
		ut_asserteq(1, priv->uclass_total);
	}
	ut_asserteq(3, device_get_child_count(bus));

	return 0;
}
DM_TEST(dm_test_bus_child_pre_probe_uclass,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/*
 * Test that the bus' uclass' child_post_probe() is called after the
 * device's probe() method
 */
static int dm_test_bus_child_post_probe_uclass(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;

	/*
	 * See testfdt_drv_probe() which effectively initializes that
	 * the uclass postp flag is set to a value
	 */
	ut_assertok(uclass_get_device(UCLASS_TEST_BUS, 0, &bus));
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		struct dm_test_priv *priv = dev_get_priv(dev);

		/* Check that things happened in the right order */
		ut_asserteq_ptr(NULL, priv);
		ut_assertok(device_probe(dev));

		priv = dev_get_priv(dev);
		ut_assert(priv != NULL);
		ut_asserteq(0, priv->uclass_postp);
	}
	ut_asserteq(3, device_get_child_count(bus));

	return 0;
}
DM_TEST(dm_test_bus_child_post_probe_uclass,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
