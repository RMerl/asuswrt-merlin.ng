// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <dm/test.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/device-internal.h>
#include <dm/devres.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <dm/of_access.h>
#include <linux/ioport.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

struct dm_testprobe_pdata {
	int probe_err;
};

static int testprobe_drv_probe(struct udevice *dev)
{
	struct dm_testprobe_pdata *pdata = dev_get_plat(dev);

	return pdata->probe_err;
}

static const struct udevice_id testprobe_ids[] = {
	{ .compatible = "denx,u-boot-probe-test" },
	{ }
};

U_BOOT_DRIVER(testprobe_drv) = {
	.name	= "testprobe_drv",
	.of_match	= testprobe_ids,
	.id	= UCLASS_TEST_PROBE,
	.probe	= testprobe_drv_probe,
	.plat_auto	= sizeof(struct dm_testprobe_pdata),
};

UCLASS_DRIVER(testprobe) = {
	.name		= "testprobe",
	.id		= UCLASS_TEST_PROBE,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

struct dm_testdevres_pdata {
	void *ptr;
};

struct dm_testdevres_priv {
	void *ptr;
	void *ptr_ofdata;
};

static int testdevres_drv_bind(struct udevice *dev)
{
	struct dm_testdevres_pdata *pdata = dev_get_plat(dev);

	pdata->ptr = devm_kmalloc(dev, TEST_DEVRES_SIZE, 0);

	return 0;
}

static int testdevres_drv_of_to_plat(struct udevice *dev)
{
	struct dm_testdevres_priv *priv = dev_get_priv(dev);

	priv->ptr_ofdata = devm_kmalloc(dev, TEST_DEVRES_SIZE3, 0);

	return 0;
}

static int testdevres_drv_probe(struct udevice *dev)
{
	struct dm_testdevres_priv *priv = dev_get_priv(dev);

	priv->ptr = devm_kmalloc(dev, TEST_DEVRES_SIZE2, 0);

	return 0;
}

static const struct udevice_id testdevres_ids[] = {
	{ .compatible = "denx,u-boot-devres-test" },
	{ }
};

U_BOOT_DRIVER(testdevres_drv) = {
	.name	= "testdevres_drv",
	.of_match	= testdevres_ids,
	.id	= UCLASS_TEST_DEVRES,
	.bind	= testdevres_drv_bind,
	.of_to_plat	= testdevres_drv_of_to_plat,
	.probe	= testdevres_drv_probe,
	.plat_auto	= sizeof(struct dm_testdevres_pdata),
	.priv_auto	= sizeof(struct dm_testdevres_priv),
};

UCLASS_DRIVER(testdevres) = {
	.name		= "testdevres",
	.id		= UCLASS_TEST_DEVRES,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

int dm_check_devices(struct unit_test_state *uts, int num_devices)
{
	struct udevice *dev;
	int ret;
	int i;

	/*
	 * Now check that the ping adds are what we expect. This is using the
	 * ping-add property in each node.
	 */
	for (i = 0; i < num_devices; i++) {
		uint32_t base;

		ret = uclass_get_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);

		/*
		 * Get the 'ping-expect' property, which tells us what the
		 * ping add should be. We don't use the plat because we
		 * want to test the code that sets that up
		 * (testfdt_drv_probe()).
		 */
		base = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				      "ping-expect", -1);
		debug("dev=%d, base=%d: %s\n", i, base,
		      fdt_get_name(gd->fdt_blob, dev_of_offset(dev), NULL));

		ut_assert(!dm_check_operations(uts, dev, base,
					       dev_get_priv(dev)));
	}

	return 0;
}

/* Test that FDT-based binding works correctly */
static int dm_test_fdt(struct unit_test_state *uts)
{
	const int num_devices = 9;
	struct udevice *dev;
	struct uclass *uc;
	int ret;
	int i;

	ret = dm_extended_scan(false);
	ut_assert(!ret);

	ret = uclass_get(UCLASS_TEST_FDT, &uc);
	ut_assert(!ret);

	/* These are num_devices compatible root-level device tree nodes */
	ut_asserteq(num_devices, list_count_items(&uc->dev_head));

	/* Each should have platform data but no private data */
	for (i = 0; i < num_devices; i++) {
		ret = uclass_find_device(UCLASS_TEST_FDT, i, &dev);
		ut_assert(!ret);
		ut_assert(!dev_get_priv(dev));
		ut_assert(dev_get_plat(dev));
	}

	ut_assertok(dm_check_devices(uts, num_devices));

	return 0;
}
DM_TEST(dm_test_fdt, 0);

static int dm_test_alias_highest_id(struct unit_test_state *uts)
{
	int ret;

	ret = dev_read_alias_highest_id("ethernet");
	ut_asserteq(8, ret);

	ret = dev_read_alias_highest_id("gpio");
	ut_asserteq(3, ret);

	ret = dev_read_alias_highest_id("pci");
	ut_asserteq(2, ret);

	ret = dev_read_alias_highest_id("i2c");
	ut_asserteq(0, ret);

	ret = dev_read_alias_highest_id("deadbeef");
	ut_asserteq(-1, ret);

	return 0;
}
DM_TEST(dm_test_alias_highest_id, 0);

static int dm_test_fdt_pre_reloc(struct unit_test_state *uts)
{
	struct uclass *uc;
	int ret;

	ret = dm_scan_fdt(true);
	ut_assert(!ret);

	ret = uclass_get(UCLASS_TEST_FDT, &uc);
	ut_assert(!ret);

	/*
	 * These are 2 pre-reloc devices:
	 * one with "bootph-all" property (a-test node), and the other
	 * one whose driver marked with DM_FLAG_PRE_RELOC flag (h-test node).
	 */
	ut_asserteq(2, list_count_items(&uc->dev_head));

	return 0;
}
DM_TEST(dm_test_fdt_pre_reloc, 0);

/* Test that sequence numbers are allocated properly */
static int dm_test_fdt_uclass_seq(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* A few basic santiy tests */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 3, &dev));
	ut_asserteq_str("b-test", dev->name);
	ut_asserteq(3, dev_seq(dev));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 8, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_asserteq(8, dev_seq(dev));

	/*
	 * This device has no alias so gets the next value after all available
	 * aliases. The last alias is testfdt12
	 */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_FDT, 13, &dev));
	ut_asserteq_str("d-test", dev->name);
	ut_asserteq(13, dev_seq(dev));

	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 9,
						       &dev));
	ut_asserteq_ptr(NULL, dev);

	/* Test aliases */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 6, &dev));
	ut_asserteq_str("e-test", dev->name);
	ut_asserteq(6, dev_seq(dev));

	/*
	 * Note that c-test nodes are not probed since it is not a top-level
	 * node
	 */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 3, &dev));
	ut_asserteq_str("b-test", dev->name);
	ut_asserteq(3, dev_seq(dev));

	/*
	 * d-test wants sequence number 3 also, but it can't have it because
	 * b-test gets it first.
	 */
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT, 2, &dev));
	ut_asserteq_str("d-test", dev->name);
	ut_asserteq(13, dev_seq(dev));

	/* g-test gets the next value after f-test */
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 15, &dev));
	ut_asserteq_str("g-test", dev->name);
	ut_asserteq(15, dev_seq(dev));

	/* And we should still have holes in our sequence numbers */
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 0,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 1,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 2,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 4,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 7,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 9,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 10,
						       &dev));
	ut_asserteq(-ENODEV, uclass_find_device_by_seq(UCLASS_TEST_FDT, 11,
						       &dev));

	return 0;
}
DM_TEST(dm_test_fdt_uclass_seq, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* More tests for sequence numbers */
static int dm_test_fdt_uclass_seq_manual(struct unit_test_state *uts)
{
	struct udevice *dev;

	/*
	 * Since DM_UC_FLAG_NO_AUTO_SEQ is set for this uclass, only testfdtm1
	 * should get a sequence number assigned
	 */
	ut_assertok(uclass_get_device(UCLASS_TEST_FDT_MANUAL, 0, &dev));
	ut_asserteq_str("testfdtm0", dev->name);
	ut_asserteq(-1, dev_seq(dev));

	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT_MANUAL, 1, &dev));
	ut_asserteq_str("testfdtm1", dev->name);
	ut_asserteq(1, dev_seq(dev));

	ut_assertok(uclass_get_device(UCLASS_TEST_FDT_MANUAL, 2, &dev));
	ut_asserteq_str("testfdtm2", dev->name);
	ut_asserteq(-1, dev_seq(dev));

	return 0;
}
DM_TEST(dm_test_fdt_uclass_seq_manual, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_fdt_uclass_seq_more(struct unit_test_state *uts)
{
	struct udevice *dev;
	ofnode node;

	/* Check creating a device with an alias */
	node = ofnode_path("/some-bus/c-test@1");
	ut_assertok(device_bind(dm_root(), DM_DRIVER_GET(denx_u_boot_fdt_test),
				"c-test@1", NULL, node, &dev));
	ut_asserteq(12, dev_seq(dev));
	ut_assertok(uclass_get_device_by_seq(UCLASS_TEST_FDT, 12, &dev));
	ut_asserteq_str("c-test@1", dev->name);

	/*
	 * Now bind a device without an alias. It should not get the next
	 * sequence number after all aliases, and existing bound devices. The
	 * last alias is 12, so we have:
	 *
	 * 13 d-test
	 * 14 f-test
	 * 15 g-test
	 * 16 h-test
	 * 17 another-test
	 * 18 chosen-test
	 *
	 * So next available is 19
	 */
	ut_assertok(device_bind(dm_root(), DM_DRIVER_GET(denx_u_boot_fdt_test),
				"fred", NULL, ofnode_null(), &dev));
	ut_asserteq(19, dev_seq(dev));

	ut_assertok(device_bind(dm_root(), DM_DRIVER_GET(denx_u_boot_fdt_test),
				"fred2", NULL, ofnode_null(), &dev));
	ut_asserteq(20, dev_seq(dev));

	return 0;
}
DM_TEST(dm_test_fdt_uclass_seq_more, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that we can find a device by device tree offset */
static int dm_test_fdt_offset(struct unit_test_state *uts)
{
	const void *blob = gd->fdt_blob;
	struct udevice *dev;
	int node;

	node = fdt_path_offset(blob, "/e-test");
	ut_assert(node > 0);
	ut_assertok(uclass_get_device_by_of_offset(UCLASS_TEST_FDT, node,
						   &dev));
	ut_asserteq_str("e-test", dev->name);

	/* This node should not be bound */
	node = fdt_path_offset(blob, "/junk");
	ut_assert(node > 0);
	ut_asserteq(-ENODEV, uclass_get_device_by_of_offset(UCLASS_TEST_FDT,
							    node, &dev));

	/* This is not a top level node so should not be probed */
	node = fdt_path_offset(blob, "/some-bus/c-test@5");
	ut_assert(node > 0);
	ut_asserteq(-ENODEV, uclass_get_device_by_of_offset(UCLASS_TEST_FDT,
							    node, &dev));

	return 0;
}
DM_TEST(dm_test_fdt_offset,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

/**
 * Test various error conditions with uclass_first_device(),
 * uclass_next_device(), and uclass_probe_all()
 */
static int dm_test_first_next_device_probeall(struct unit_test_state *uts)
{
	struct dm_testprobe_pdata *pdata;
	struct udevice *dev, *parent = NULL;
	int count;
	int ret;

	/* There should be 4 devices */
	for (uclass_first_device(UCLASS_TEST_PROBE, &dev), count = 0;
	     dev;
	     uclass_next_device(&dev)) {
		count++;
		parent = dev_get_parent(dev);
		}
	ut_asserteq(4, count);

	/* Remove them and try again, with an error on the second one */
	ut_assertok(uclass_get_device(UCLASS_TEST_PROBE, 1, &dev));
	pdata = dev_get_plat(dev);
	pdata->probe_err = -ENOMEM;
	device_remove(parent, DM_REMOVE_NORMAL);
	for (ret = uclass_first_device_check(UCLASS_TEST_PROBE, &dev),
		count = 0;
	     dev;
	     ret = uclass_next_device_check(&dev)) {
		if (!ret)
			count++;
		else
			ut_asserteq(-ENOMEM, ret);
		parent = dev_get_parent(dev);
		}
	ut_asserteq(3, count);

	/* Now an error on the first one */
	ut_assertok(uclass_get_device(UCLASS_TEST_PROBE, 0, &dev));
	pdata = dev_get_plat(dev);
	pdata->probe_err = -ENOENT;
	device_remove(parent, DM_REMOVE_NORMAL);
	for (uclass_first_device(UCLASS_TEST_PROBE, &dev), count = 0;
	     dev;
	     uclass_next_device(&dev)) {
		count++;
		parent = dev_get_parent(dev);
		}
	ut_asserteq(2, count);

	/* Now that broken devices are set up test probe_all */
	device_remove(parent, DM_REMOVE_NORMAL);
	/* There are broken devices so an error should be returned */
	ut_assert(uclass_probe_all(UCLASS_TEST_PROBE) < 0);
	/* but non-error device should be probed nonetheless */
	ut_assertok(uclass_get_device(UCLASS_TEST_PROBE, 2, &dev));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);
	ut_assertok(uclass_get_device(UCLASS_TEST_PROBE, 3, &dev));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);

	return 0;
}
DM_TEST(dm_test_first_next_device_probeall,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test iteration through devices in a uclass */
static int dm_test_uclass_foreach(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct uclass *uc;
	int count;

	count = 0;
	uclass_id_foreach_dev(UCLASS_TEST_FDT, dev, uc)
		count++;
	ut_asserteq(9, count);

	count = 0;
	uclass_foreach_dev(dev, uc)
		count++;
	ut_asserteq(9, count);

	return 0;
}
DM_TEST(dm_test_uclass_foreach, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/**
 * check_devices() - Check return values and pointers
 *
 * This runs through a full sequence of uclass_first_device_check()...
 * uclass_next_device_check() checking that the return values and devices
 * are correct.
 *
 * @uts: Test state
 * @devlist: List of expected devices
 * @mask: Indicates which devices should return an error. Device n should
 *	  return error (-NOENT - n) if bit n is set, or no error (i.e. 0) if
 *	  bit n is clear.
 */
static int check_devices(struct unit_test_state *uts,
			 struct udevice *devlist[], int mask)
{
	int expected_ret;
	struct udevice *dev;
	int i;

	expected_ret = (mask & 1) ? -ENOENT : 0;
	mask >>= 1;
	ut_asserteq(expected_ret,
		    uclass_first_device_check(UCLASS_TEST_PROBE, &dev));
	for (i = 0; i < 4; i++) {
		ut_asserteq_ptr(devlist[i], dev);
		expected_ret = (mask & 1) ? -ENOENT - (i + 1) : 0;
		mask >>= 1;
		ut_asserteq(expected_ret, uclass_next_device_check(&dev));
	}
	ut_asserteq_ptr(NULL, dev);

	return 0;
}

/* Test uclass_first_device_check() and uclass_next_device_check() */
static int dm_test_first_next_ok_device(struct unit_test_state *uts)
{
	struct dm_testprobe_pdata *pdata;
	struct udevice *dev, *parent = NULL, *devlist[4];
	int count;
	int ret;

	/* There should be 4 devices */
	count = 0;
	for (ret = uclass_first_device_check(UCLASS_TEST_PROBE, &dev);
	     dev;
	     ret = uclass_next_device_check(&dev)) {
		ut_assertok(ret);
		devlist[count++] = dev;
		parent = dev_get_parent(dev);
		}
	ut_asserteq(4, count);
	ut_assertok(uclass_first_device_check(UCLASS_TEST_PROBE, &dev));
	ut_assertok(check_devices(uts, devlist, 0));

	/* Remove them and try again, with an error on the second one */
	pdata = dev_get_plat(devlist[1]);
	pdata->probe_err = -ENOENT - 1;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(check_devices(uts, devlist, 1 << 1));

	/* Now an error on the first one */
	pdata = dev_get_plat(devlist[0]);
	pdata->probe_err = -ENOENT - 0;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(check_devices(uts, devlist, 3 << 0));

	/* Now errors on all */
	pdata = dev_get_plat(devlist[2]);
	pdata->probe_err = -ENOENT - 2;
	pdata = dev_get_plat(devlist[3]);
	pdata->probe_err = -ENOENT - 3;
	device_remove(parent, DM_REMOVE_NORMAL);
	ut_assertok(check_devices(uts, devlist, 0xf << 0));

	return 0;
}
DM_TEST(dm_test_first_next_ok_device, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static const struct udevice_id fdt_dummy_ids[] = {
	{ .compatible = "denx,u-boot-fdt-dummy", },
	{ }
};

UCLASS_DRIVER(fdt_dummy) = {
	.name		= "fdt-dummy",
	.id		= UCLASS_TEST_DUMMY,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
};

U_BOOT_DRIVER(fdt_dummy_drv) = {
	.name	= "fdt_dummy_drv",
	.of_match	= fdt_dummy_ids,
	.id	= UCLASS_TEST_DUMMY,
};

static int dm_test_fdt_translation(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt32_t dma_addr[2];

	/* Some simple translations */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));
	ut_asserteq_str("dev@0,0", dev->name);
	ut_asserteq(0x8000, dev_read_addr(dev));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 1, &dev));
	ut_asserteq_str("dev@1,100", dev->name);
	ut_asserteq(0x9000, dev_read_addr(dev));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 2, &dev));
	ut_asserteq_str("dev@2,200", dev->name);
	ut_asserteq(0xA000, dev_read_addr(dev));

	/* No translation for buses with #size-cells == 0 */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 3, &dev));
	ut_asserteq_str("dev@42", dev->name);
	ut_asserteq(0x42, dev_read_addr(dev));

	/* dma address translation */
	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));
	dma_addr[0] = cpu_to_be32(0);
	dma_addr[1] = cpu_to_be32(0);
	ut_asserteq(0x10000000, dev_translate_dma_address(dev, dma_addr));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 1, &dev));
	dma_addr[0] = cpu_to_be32(1);
	dma_addr[1] = cpu_to_be32(0x100);
	ut_asserteq(0x20000000, dev_translate_dma_address(dev, dma_addr));

	return 0;
}
DM_TEST(dm_test_fdt_translation, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_fdt_get_addr_ptr_flat(struct unit_test_state *uts)
{
	struct udevice *gpio, *dev;
	void *ptr;
	void *paddr;

	/* Test for missing reg property */
	ut_assertok(uclass_first_device_err(UCLASS_GPIO, &gpio));
	ut_assertnull(devfdt_get_addr_ptr(gpio));

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));
	ptr = devfdt_get_addr_ptr(dev);

	paddr = map_sysmem(0x8000, 0);
	ut_asserteq_ptr(paddr, ptr);

	return 0;
}
DM_TEST(dm_test_fdt_get_addr_ptr_flat,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

static int dm_test_fdt_remap_addr_flat(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));

	addr = devfdt_get_addr(dev);
	ut_asserteq(0x8000, addr);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, devfdt_remap_addr(dev));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_flat,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

static int dm_test_fdt_remap_addr_index_flat(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	fdt_size_t size;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));

	addr = devfdt_get_addr_size_index(dev, 0, &size);
	ut_asserteq(0x8000, addr);
	ut_asserteq(0x1000, size);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, devfdt_remap_addr_index(dev, 0));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_index_flat,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

static int dm_test_fdt_remap_addr_name_flat(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	fdt_size_t size;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));

	addr = devfdt_get_addr_size_name(dev, "sandbox-dummy-0", &size);
	ut_asserteq(0x8000, addr);
	ut_asserteq(0x1000, size);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, devfdt_remap_addr_name(dev, "sandbox-dummy-0"));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_name_flat,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT | UT_TESTF_FLAT_TREE);

static int dm_test_fdt_remap_addr_live(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));

	addr = dev_read_addr(dev);
	ut_asserteq(0x8000, addr);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, dev_remap_addr(dev));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_live,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_fdt_remap_addr_index_live(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	fdt_size_t size;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));

	addr = dev_read_addr_size_index(dev, 0, &size);
	ut_asserteq(0x8000, addr);
	ut_asserteq(0x1000, size);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, dev_remap_addr_index(dev, 0));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_index_live,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_fdt_remap_addr_name_live(struct unit_test_state *uts)
{
	struct udevice *dev;
	fdt_addr_t addr;
	fdt_size_t size;
	void *paddr;

	ut_assertok(uclass_find_device_by_seq(UCLASS_TEST_DUMMY, 0, &dev));

	addr = dev_read_addr_size_name(dev, "sandbox-dummy-0", &size);
	ut_asserteq(0x8000, addr);
	ut_asserteq(0x1000, size);

	paddr = map_physmem(addr, 0, MAP_NOCACHE);
	ut_assertnonnull(paddr);
	ut_asserteq_ptr(paddr, dev_remap_addr_name(dev, "sandbox-dummy-0"));

	return 0;
}
DM_TEST(dm_test_fdt_remap_addr_name_live,
	UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_fdt_disable_enable_by_path(struct unit_test_state *uts)
{
	ofnode node;

	if (!of_live_active()) {
		printf("Live tree not active; ignore test\n");
		return 0;
	}

	node = ofnode_path("/usb@2");

	/* Test enabling devices */

	ut_assert(!of_device_is_available(ofnode_to_np(node)));
	dev_enable_by_path("/usb@2");
	ut_assert(of_device_is_available(ofnode_to_np(node)));

	/* Test disabling devices */

	ut_assert(of_device_is_available(ofnode_to_np(node)));
	dev_disable_by_path("/usb@2");
	ut_assert(!of_device_is_available(ofnode_to_np(node)));

	return 0;
}
DM_TEST(dm_test_fdt_disable_enable_by_path, UT_TESTF_SCAN_PDATA |
					    UT_TESTF_SCAN_FDT);

/* Test a few uclass phandle functions */
static int dm_test_fdt_phandle(struct unit_test_state *uts)
{
	struct udevice *back, *dev, *dev2;

	ut_assertok(uclass_find_first_device(UCLASS_PANEL_BACKLIGHT, &back));
	ut_assertnonnull(back);
	ut_asserteq(-ENOENT, uclass_find_device_by_phandle(UCLASS_REGULATOR,
							back, "missing", &dev));
	ut_assertok(uclass_find_device_by_phandle(UCLASS_REGULATOR, back,
						  "power-supply", &dev));
	ut_assertnonnull(dev);
	ut_asserteq(0, device_active(dev));
	ut_asserteq_str("ldo1", dev->name);
	ut_assertok(uclass_get_device_by_phandle(UCLASS_REGULATOR, back,
						 "power-supply", &dev2));
	ut_asserteq_ptr(dev, dev2);

	return 0;
}
DM_TEST(dm_test_fdt_phandle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test device_find_first_child_by_uclass() */
static int dm_test_first_child(struct unit_test_state *uts)
{
	struct udevice *i2c, *dev, *dev2;

	ut_assertok(uclass_first_device_err(UCLASS_I2C, &i2c));
	ut_assertok(device_find_first_child_by_uclass(i2c, UCLASS_RTC, &dev));
	ut_asserteq_str("rtc@43", dev->name);
	ut_assertok(device_find_child_by_name(i2c, "rtc@43", &dev2));
	ut_asserteq_ptr(dev, dev2);
	ut_assertok(device_find_child_by_name(i2c, "rtc@61", &dev2));
	ut_asserteq_str("rtc@61", dev2->name);

	ut_assertok(device_find_first_child_by_uclass(i2c, UCLASS_I2C_EEPROM,
						      &dev));
	ut_asserteq_str("eeprom@2c", dev->name);
	ut_assertok(device_find_child_by_name(i2c, "eeprom@2c", &dev2));
	ut_asserteq_ptr(dev, dev2);

	ut_asserteq(-ENODEV, device_find_first_child_by_uclass(i2c,
							UCLASS_VIDEO, &dev));
	ut_asserteq(-ENODEV, device_find_child_by_name(i2c, "missing", &dev));

	return 0;
}
DM_TEST(dm_test_first_child, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test integer functions in dm_read_...() */
static int dm_test_read_int(struct unit_test_state *uts)
{
	struct udevice *dev;
	u8 val8;
	u16 val16;
	u32 val32;
	s32 sval;
	uint val;
	u64 val64;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(dev_read_u8(dev, "int8-value", &val8));
	ut_asserteq(0x12, val8);

	ut_asserteq(-EINVAL, dev_read_u8(dev, "missing", &val8));
	ut_asserteq(6, dev_read_u8_default(dev, "missing", 6));

	ut_asserteq(0x12, dev_read_u8_default(dev, "int8-value", 6));

	ut_assertok(dev_read_u16(dev, "int16-value", &val16));
	ut_asserteq(0x1234, val16);

	ut_asserteq(-EINVAL, dev_read_u16(dev, "missing", &val16));
	ut_asserteq(6, dev_read_u16_default(dev, "missing", 6));

	ut_asserteq(0x1234, dev_read_u16_default(dev, "int16-value", 6));

	ut_assertok(dev_read_u32(dev, "int-value", &val32));
	ut_asserteq(1234, val32);

	ut_asserteq(-EINVAL, dev_read_u32(dev, "missing", &val32));
	ut_asserteq(6, dev_read_u32_default(dev, "missing", 6));

	ut_asserteq(1234, dev_read_u32_default(dev, "int-value", 6));
	ut_asserteq(1234, val32);

	ut_asserteq(-EINVAL, dev_read_s32(dev, "missing", &sval));
	ut_asserteq(6, dev_read_s32_default(dev, "missing", 6));

	ut_asserteq(-1234, dev_read_s32_default(dev, "uint-value", 6));
	ut_assertok(dev_read_s32(dev, "uint-value", &sval));
	ut_asserteq(-1234, sval);

	val = 0;
	ut_asserteq(-EINVAL, dev_read_u32u(dev, "missing", &val));
	ut_assertok(dev_read_u32u(dev, "uint-value", &val));
	ut_asserteq(-1234, val);

	ut_assertok(dev_read_u64(dev, "int64-value", &val64));
	ut_asserteq_64(0x1111222233334444, val64);

	ut_asserteq_64(-EINVAL, dev_read_u64(dev, "missing", &val64));
	ut_asserteq_64(6, dev_read_u64_default(dev, "missing", 6));

	ut_asserteq_64(0x1111222233334444,
		       dev_read_u64_default(dev, "int64-value", 6));

	return 0;
}
DM_TEST(dm_test_read_int, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_read_int_index(struct unit_test_state *uts)
{
	struct udevice *dev;
	u32 val32;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_asserteq(-EINVAL, dev_read_u32_index(dev, "missing", 0, &val32));
	ut_asserteq(19, dev_read_u32_index_default(dev, "missing", 0, 19));

	ut_assertok(dev_read_u32_index(dev, "int-array", 0, &val32));
	ut_asserteq(5678, val32);
	ut_assertok(dev_read_u32_index(dev, "int-array", 1, &val32));
	ut_asserteq(9123, val32);
	ut_assertok(dev_read_u32_index(dev, "int-array", 2, &val32));
	ut_asserteq(4567, val32);
	ut_asserteq(-EOVERFLOW, dev_read_u32_index(dev, "int-array", 3,
						   &val32));

	ut_asserteq(5678, dev_read_u32_index_default(dev, "int-array", 0, 2));
	ut_asserteq(9123, dev_read_u32_index_default(dev, "int-array", 1, 2));
	ut_asserteq(4567, dev_read_u32_index_default(dev, "int-array", 2, 2));
	ut_asserteq(2, dev_read_u32_index_default(dev, "int-array", 3, 2));

	return 0;
}
DM_TEST(dm_test_read_int_index, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_read_phandle(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct ofnode_phandle_args args;
	int ret;
	const char prop[] = "test-gpios";
	const char cell[] = "#gpio-cells";
	const char prop2[] = "phandle-value";

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);

	/* Test dev_count_phandle_with_args with cell name */
	ret = dev_count_phandle_with_args(dev, "missing", cell, 0);
	ut_asserteq(-ENOENT, ret);
	ret = dev_count_phandle_with_args(dev, prop, "#invalid", 0);
	ut_asserteq(-EINVAL, ret);
	ut_asserteq(5, dev_count_phandle_with_args(dev, prop, cell, 0));

	/* Test dev_read_phandle_with_args with cell name */
	ret = dev_read_phandle_with_args(dev, "missing", cell, 0, 0, &args);
	ut_asserteq(-ENOENT, ret);
	ret = dev_read_phandle_with_args(dev, prop, "#invalid", 0, 0, &args);
	ut_asserteq(-EINVAL, ret);
	ut_assertok(dev_read_phandle_with_args(dev, prop, cell, 0, 0, &args));
	ut_asserteq(1, args.args_count);
	ut_asserteq(1, args.args[0]);
	ut_assertok(dev_read_phandle_with_args(dev, prop, cell, 0, 1, &args));
	ut_asserteq(1, args.args_count);
	ut_asserteq(4, args.args[0]);
	ut_assertok(dev_read_phandle_with_args(dev, prop, cell, 0, 2, &args));
	ut_asserteq(5, args.args_count);
	ut_asserteq(5, args.args[0]);
	ut_asserteq(1, args.args[4]);
	ret = dev_read_phandle_with_args(dev, prop, cell, 0, 3, &args);
	ut_asserteq(-ENOENT, ret);
	ut_assertok(dev_read_phandle_with_args(dev, prop, cell, 0, 4, &args));
	ut_asserteq(1, args.args_count);
	ut_asserteq(12, args.args[0]);
	ret = dev_read_phandle_with_args(dev, prop, cell, 0, 5, &args);
	ut_asserteq(-ENOENT, ret);

	/* Test dev_count_phandle_with_args with cell count */
	ret = dev_count_phandle_with_args(dev, "missing", NULL, 2);
	ut_asserteq(-ENOENT, ret);
	ut_asserteq(3, dev_count_phandle_with_args(dev, prop2, NULL, 1));

	/* Test dev_read_phandle_with_args with cell count */
	ut_assertok(dev_read_phandle_with_args(dev, prop2, NULL, 1, 0, &args));
	ut_asserteq(1, ofnode_valid(args.node));
	ut_asserteq(1, args.args_count);
	ut_asserteq(10, args.args[0]);
	ret = dev_read_phandle_with_args(dev, prop2, NULL, 1, 1, &args);
	ut_asserteq(-EINVAL, ret);
	ut_assertok(dev_read_phandle_with_args(dev, prop2, NULL, 1, 2, &args));
	ut_asserteq(1, ofnode_valid(args.node));
	ut_asserteq(1, args.args_count);
	ut_asserteq(30, args.args[0]);
	ret = dev_read_phandle_with_args(dev, prop2, NULL, 1, 3, &args);
	ut_asserteq(-ENOENT, ret);

	return 0;
}
DM_TEST(dm_test_read_phandle, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test iteration through devices by drvdata */
static int dm_test_uclass_drvdata(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_drvdata(UCLASS_TEST_FDT,
						DM_TEST_TYPE_FIRST, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(uclass_first_device_drvdata(UCLASS_TEST_FDT,
						DM_TEST_TYPE_SECOND, &dev));
	ut_asserteq_str("d-test", dev->name);

	ut_asserteq(-ENODEV, uclass_first_device_drvdata(UCLASS_TEST_FDT,
							 DM_TEST_TYPE_COUNT,
							 &dev));

	return 0;
}
DM_TEST(dm_test_uclass_drvdata, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test device_first_child_ofdata_err(), etc. */
static int dm_test_child_ofdata(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	int count;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_BUS, &bus));
	count = 0;
	device_foreach_child_of_to_plat(dev, bus) {
		ut_assert(dev_get_flags(dev) & DM_FLAG_PLATDATA_VALID);
		ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));
		count++;
	}
	ut_asserteq(3, count);

	return 0;
}
DM_TEST(dm_test_child_ofdata, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test device_first_child_err(), etc. */
static int dm_test_first_child_probe(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;
	int count;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_BUS, &bus));
	count = 0;
	device_foreach_child_probe(dev, bus) {
		ut_assert(dev_get_flags(dev) & DM_FLAG_PLATDATA_VALID);
		ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);
		count++;
	}
	ut_asserteq(3, count);

	return 0;
}
DM_TEST(dm_test_first_child_probe, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test that ofdata is read for parents before children */
static int dm_test_ofdata_order(struct unit_test_state *uts)
{
	struct udevice *bus, *dev;

	ut_assertok(uclass_find_first_device(UCLASS_I2C, &bus));
	ut_assertnonnull(bus);
	ut_assert(!(dev_get_flags(bus) & DM_FLAG_PLATDATA_VALID));

	ut_assertok(device_find_first_child(bus, &dev));
	ut_assertnonnull(dev);
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_PLATDATA_VALID));

	/* read the child's ofdata which should cause the parent's to be read */
	ut_assertok(device_of_to_plat(dev));
	ut_assert(dev_get_flags(dev) & DM_FLAG_PLATDATA_VALID);
	ut_assert(dev_get_flags(bus) & DM_FLAG_PLATDATA_VALID);

	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));
	ut_assert(!(dev_get_flags(bus) & DM_FLAG_ACTIVATED));

	return 0;
}
DM_TEST(dm_test_ofdata_order, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test dev_decode_display_timing() */
static int dm_test_decode_display_timing(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct display_timing timing;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(dev_decode_display_timing(dev, 0, &timing));
	ut_assert(timing.hactive.typ == 240);
	ut_assert(timing.hback_porch.typ == 7);
	ut_assert(timing.hfront_porch.typ == 6);
	ut_assert(timing.hsync_len.typ == 1);
	ut_assert(timing.vactive.typ == 320);
	ut_assert(timing.vback_porch.typ == 5);
	ut_assert(timing.vfront_porch.typ == 8);
	ut_assert(timing.vsync_len.typ == 2);
	ut_assert(timing.pixelclock.typ == 6500000);
	ut_assert(timing.flags & DISPLAY_FLAGS_HSYNC_HIGH);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_HSYNC_LOW));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_VSYNC_HIGH));
	ut_assert(timing.flags & DISPLAY_FLAGS_VSYNC_LOW);
	ut_assert(timing.flags & DISPLAY_FLAGS_DE_HIGH);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DE_LOW));
	ut_assert(timing.flags & DISPLAY_FLAGS_PIXDATA_POSEDGE);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE));
	ut_assert(timing.flags & DISPLAY_FLAGS_INTERLACED);
	ut_assert(timing.flags & DISPLAY_FLAGS_DOUBLESCAN);
	ut_assert(timing.flags & DISPLAY_FLAGS_DOUBLECLK);

	ut_assertok(dev_decode_display_timing(dev, 1, &timing));
	ut_assert(timing.hactive.typ == 480);
	ut_assert(timing.hback_porch.typ == 59);
	ut_assert(timing.hfront_porch.typ == 10);
	ut_assert(timing.hsync_len.typ == 12);
	ut_assert(timing.vactive.typ == 800);
	ut_assert(timing.vback_porch.typ == 15);
	ut_assert(timing.vfront_porch.typ == 17);
	ut_assert(timing.vsync_len.typ == 16);
	ut_assert(timing.pixelclock.typ == 9000000);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_HSYNC_HIGH));
	ut_assert(timing.flags & DISPLAY_FLAGS_HSYNC_LOW);
	ut_assert(timing.flags & DISPLAY_FLAGS_VSYNC_HIGH);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_VSYNC_LOW));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DE_HIGH));
	ut_assert(timing.flags & DISPLAY_FLAGS_DE_LOW);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_PIXDATA_POSEDGE));
	ut_assert(timing.flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_INTERLACED));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DOUBLESCAN));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DOUBLECLK));

	ut_assertok(dev_decode_display_timing(dev, 2, &timing));
	ut_assert(timing.hactive.typ == 800);
	ut_assert(timing.hback_porch.typ == 89);
	ut_assert(timing.hfront_porch.typ == 164);
	ut_assert(timing.hsync_len.typ == 11);
	ut_assert(timing.vactive.typ == 480);
	ut_assert(timing.vback_porch.typ == 23);
	ut_assert(timing.vfront_porch.typ == 10);
	ut_assert(timing.vsync_len.typ == 13);
	ut_assert(timing.pixelclock.typ == 33500000);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_HSYNC_HIGH));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_HSYNC_LOW));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_VSYNC_HIGH));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_VSYNC_LOW));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DE_HIGH));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DE_LOW));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_PIXDATA_POSEDGE));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_INTERLACED));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DOUBLESCAN));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DOUBLECLK));

	ut_assert(dev_decode_display_timing(dev, 3, &timing));
	return 0;
}
DM_TEST(dm_test_decode_display_timing, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test dev_decode_panel_timing() */
static int dm_test_decode_panel_timing(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct display_timing timing;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);

	ut_assertok(dev_decode_panel_timing(dev, &timing));
	ut_assert(timing.hactive.typ == 240);
	ut_assert(timing.hback_porch.typ == 7);
	ut_assert(timing.hfront_porch.typ == 6);
	ut_assert(timing.hsync_len.typ == 1);
	ut_assert(timing.vactive.typ == 320);
	ut_assert(timing.vback_porch.typ == 5);
	ut_assert(timing.vfront_porch.typ == 8);
	ut_assert(timing.vsync_len.typ == 2);
	ut_assert(timing.pixelclock.typ == 6500000);
	ut_assert(timing.flags & DISPLAY_FLAGS_HSYNC_HIGH);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_HSYNC_LOW));
	ut_assert(!(timing.flags & DISPLAY_FLAGS_VSYNC_HIGH));
	ut_assert(timing.flags & DISPLAY_FLAGS_VSYNC_LOW);
	ut_assert(timing.flags & DISPLAY_FLAGS_DE_HIGH);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_DE_LOW));
	ut_assert(timing.flags & DISPLAY_FLAGS_PIXDATA_POSEDGE);
	ut_assert(!(timing.flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE));
	ut_assert(timing.flags & DISPLAY_FLAGS_INTERLACED);
	ut_assert(timing.flags & DISPLAY_FLAGS_DOUBLESCAN);
	ut_assert(timing.flags & DISPLAY_FLAGS_DOUBLECLK);

	return 0;
}
DM_TEST(dm_test_decode_panel_timing, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

/* Test read_resourcee() */
static int dm_test_read_resource(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct resource res;

	/* test resource without translation */
	ut_assertok(uclass_find_device_by_name(UCLASS_SIMPLE_BUS, "syscon@2", &dev));
	ut_assertok(dev_read_resource(dev, 0, &res));
	ut_asserteq(0x40, res.start);
	ut_asserteq(0x44, res.end);
	ut_assertok(dev_read_resource(dev, 1, &res));
	ut_asserteq(0x48, res.start);
	ut_asserteq(0x4d, res.end);

	/* test resource with translation */
	ut_assertok(uclass_find_device_by_name(UCLASS_TEST_DUMMY, "dev@1,100", &dev));
	ut_assertok(dev_read_resource(dev, 0, &res));
	ut_asserteq(0x9000, res.start);
	ut_asserteq(0x9fff, res.end);

	/* test named resource */
	ut_assertok(uclass_find_device_by_name(UCLASS_TEST_DUMMY, "dev@0,0", &dev));
	ut_assertok(dev_read_resource_byname(dev, "sandbox-dummy-0", &res));
	ut_asserteq(0x8000, res.start);
	ut_asserteq(0x8fff, res.end);

	return 0;
}

DM_TEST(dm_test_read_resource, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
