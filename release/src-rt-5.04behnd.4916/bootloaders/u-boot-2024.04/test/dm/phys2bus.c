// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Nicolas Saenz Julienne <nsaenzjulienne@suse.de>
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <phys2bus.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/ut.h>

static int dm_test_phys_to_bus(struct unit_test_state *uts)
{
	struct udevice *dev;
	ofnode node;

	node = ofnode_path("/mmio-bus@0");
	ut_assert(ofnode_valid(node));
	ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_BUS, node, &dev));
	/* In this case it should be transparent, no dma-ranges in parent bus */
	ut_asserteq_addr((void*)0xfffffULL, (void*)dev_phys_to_bus(dev, 0xfffff));
	ut_asserteq_addr((void*)0xfffffULL, (void*)(ulong)dev_bus_to_phys(dev, 0xfffff));

	node = ofnode_path("/mmio-bus@0/subnode@0");
	ut_assert(ofnode_valid(node));
	ut_assertok(uclass_get_device_by_ofnode(UCLASS_TEST_FDT, node, &dev));
	ut_asserteq_addr((void*)0x100fffffULL, (void*)dev_phys_to_bus(dev, 0xfffff));
	ut_asserteq_addr((void*)0xfffffULL, (void*)(ulong)dev_bus_to_phys(dev, 0x100fffff));

	return 0;
}
DM_TEST(dm_test_phys_to_bus, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);
