// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Nicolas Saenz Julienne <nsaenzjulienne@suse.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

static int dm_test_dma_ranges(struct unit_test_state *uts)
{
	struct udevice *dev;
	phys_addr_t cpu;
	dma_addr_t bus;
	ofnode node;
	u64 size;

	/* dma-ranges are on the device's node */
	node = ofnode_path("/mmio-bus@0");
	ut_assert(ofnode_valid(node));
	ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_BUS, node, &dev));
	ut_assertok(dev_get_dma_range(dev, &cpu, &bus, &size));
	ut_asserteq_64(0x40000, size);
	ut_asserteq_64(0x0, cpu);
	ut_asserteq_64(0x10000000, bus);

	/* dma-ranges are on the bus' node */
	node = ofnode_path("/mmio-bus@0/subnode@0");
	ut_assert(ofnode_valid(node));
	ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_FDT, node, &dev));
	ut_assertok(dev_get_dma_range(dev, &cpu, &bus, &size));
	ut_asserteq_64(0x40000, size);
	ut_asserteq_64(0x0, cpu);
	ut_asserteq_64(0x10000000, bus);

	/* No dma-ranges available */
	node = ofnode_path("/mmio-bus@1");
	ut_assert(ofnode_valid(node));
	ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_BUS, node, &dev));
	ut_asserteq(-ENOENT, dev_get_dma_range(dev, &cpu, &bus, &size));

	return 0;
}
DM_TEST(dm_test_dma_ranges, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
