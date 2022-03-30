// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Base test of register maps */
static int dm_test_regmap_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct regmap *map;
	ofnode node;
	int i;

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);
	ut_asserteq(1, map->range_count);
	ut_asserteq(0x10, map->ranges[0].start);
	ut_asserteq(16, map->ranges[0].size);
	ut_asserteq(0x10, map_to_sysmem(regmap_get_range(map, 0)));

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 1, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);
	ut_asserteq(4, map->range_count);
	ut_asserteq(0x20, map->ranges[0].start);
	for (i = 0; i < 4; i++) {
		const unsigned long addr = 0x20 + 8 * i;

		ut_asserteq(addr, map->ranges[i].start);
		ut_asserteq(5 + i, map->ranges[i].size);
		ut_asserteq(addr, map_to_sysmem(regmap_get_range(map, i)));
	}

	/* Check that we can't pretend a different device is a syscon */
	ut_assertok(uclass_get_device(UCLASS_I2C, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_asserteq_ptr(ERR_PTR(-ENOEXEC), map);

	/* A different device can be a syscon by using Linux-compat API */
	node = ofnode_path("/syscon@2");
	ut_assert(ofnode_valid(node));

	map = syscon_node_to_regmap(node);
	ut_assertok_ptr(map);
	ut_asserteq(4, map->range_count);
	ut_asserteq(0x40, map->ranges[0].start);
	for (i = 0; i < 4; i++) {
		const unsigned long addr = 0x40 + 8 * i;

		ut_asserteq(addr, map->ranges[i].start);
		ut_asserteq(5 + i, map->ranges[i].size);
		ut_asserteq(addr, map_to_sysmem(regmap_get_range(map, i)));
	}

	return 0;
}
DM_TEST(dm_test_regmap_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test we can access a regmap through syscon */
static int dm_test_regmap_syscon(struct unit_test_state *uts)
{
	struct regmap *map;

	map = syscon_get_regmap_by_driver_data(SYSCON0);
	ut_assertok_ptr(map);
	ut_asserteq(1, map->range_count);

	map = syscon_get_regmap_by_driver_data(SYSCON1);
	ut_assertok_ptr(map);
	ut_asserteq(4, map->range_count);

	map = syscon_get_regmap_by_driver_data(SYSCON_COUNT);
	ut_asserteq_ptr(ERR_PTR(-ENODEV), map);

	ut_asserteq(0x10, map_to_sysmem(syscon_get_first_range(SYSCON0)));
	ut_asserteq(0x20, map_to_sysmem(syscon_get_first_range(SYSCON1)));
	ut_asserteq_ptr(ERR_PTR(-ENODEV),
			syscon_get_first_range(SYSCON_COUNT));

	return 0;
}

DM_TEST(dm_test_regmap_syscon, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Read/Write/Modify test */
static int dm_test_regmap_rw(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct regmap *map;
	uint reg;

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);

	ut_assertok(regmap_write(map, 0, 0xcacafafa));
	ut_assertok(regmap_write(map, 3, 0x55aa2211));

	ut_assertok(regmap_read(map, 0, &reg));
	ut_assertok(regmap_read(map, 3, &reg));

	ut_assertok(regmap_update_bits(map, 0, 0xff00ff00, 0x55aa2211));
	ut_assertok(regmap_update_bits(map, 3, 0x00ff00ff, 0xcacafada));

	return 0;
}

DM_TEST(dm_test_regmap_rw, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Get/Set test */
static int dm_test_regmap_getset(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct regmap *map;
	uint reg;
	struct layout {
		u32 val0;
		u32 val1;
		u32 val2;
		u32 val3;
	};

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);

	regmap_set(map, struct layout, val0, 0xcacafafa);
	regmap_set(map, struct layout, val3, 0x55aa2211);

	ut_assertok(regmap_get(map, struct layout, val0, &reg));
	ut_assertok(regmap_get(map, struct layout, val3, &reg));

	return 0;
}

DM_TEST(dm_test_regmap_getset, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Read polling test */
static int dm_test_regmap_poll(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct regmap *map;
	uint reg;
	unsigned long start;

	ut_assertok(uclass_get_device(UCLASS_SYSCON, 0, &dev));
	map = syscon_get_regmap(dev);
	ut_assertok_ptr(map);

	start = get_timer(0);

	ut_asserteq(-ETIMEDOUT,
		    regmap_read_poll_timeout_test(map, 0, reg,
						  (reg == 0xcacafafa),
						  1, 5 * CONFIG_SYS_HZ,
						  5 * CONFIG_SYS_HZ));

	ut_assert(get_timer(start) > (5 * CONFIG_SYS_HZ));

	return 0;
}

DM_TEST(dm_test_regmap_poll, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
