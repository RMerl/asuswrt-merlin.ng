// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <axi.h>
#include <dm.h>
#include <dm/test.h>
#include <test/ut.h>
#include <asm/axi.h>

/* Test that sandbox AXI works correctly */
static int dm_test_axi_base(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_assertok(uclass_get_device(UCLASS_AXI, 0, &bus));

	return 0;
}

DM_TEST(dm_test_axi_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox PCI bus numbering works correctly */
static int dm_test_axi_busnum(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_assertok(uclass_get_device_by_seq(UCLASS_AXI, 0, &bus));

	return 0;
}

DM_TEST(dm_test_axi_busnum, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can use the store device correctly */
static int dm_test_axi_store(struct unit_test_state *uts)
{
	struct udevice *store;
	u8 tdata1[] = {0x55, 0x66, 0x77, 0x88};
	u8 tdata2[] = {0xaa, 0xbb, 0xcc, 0xdd};
	u32 val;
	u8 *data;

	/* Check that asking for the device automatically fires up AXI */
	ut_assertok(uclass_get_device(UCLASS_AXI_EMUL, 0, &store));
	ut_assert(device_active(store));

	axi_get_store(store, &data);

	/* Test reading */
	memcpy(data, tdata1, ARRAY_SIZE(tdata1));
	axi_read(store, 0, &val, AXI_SIZE_32);
	ut_asserteq(0x55667788, val);

	memcpy(data + 3, tdata2, ARRAY_SIZE(tdata2));
	axi_read(store, 3, &val, AXI_SIZE_32);
	ut_asserteq(0xaabbccdd, val);

	/* Reset data store */
	memset(data, 0, 16);

	/* Test writing */
	val = 0x55667788;
	axi_write(store, 0, &val, AXI_SIZE_32);
	ut_asserteq(0, memcmp(data, tdata1, ARRAY_SIZE(tdata1)));

	val = 0xaabbccdd;
	axi_write(store, 3, &val, AXI_SIZE_32);
	ut_asserteq(0, memcmp(data + 3, tdata2, ARRAY_SIZE(tdata1)));

	return 0;
}

DM_TEST(dm_test_axi_store, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
