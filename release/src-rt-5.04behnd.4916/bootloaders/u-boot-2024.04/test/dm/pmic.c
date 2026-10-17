// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the driver model pmic API
 *
 * Copyright (c) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <fsl_pmic.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <dm/util.h>
#include <power/pmic.h>
#include <power/sandbox_pmic.h>
#include <test/test.h>
#include <test/ut.h>

/* Test PMIC get method */

static inline int power_pmic_get(struct unit_test_state *uts, char *name)
{
	struct udevice *dev;

	ut_assertok(pmic_get(name, &dev));
	ut_assertnonnull(dev);

	/* Check PMIC's name */
	ut_asserteq_str(name, dev->name);

	return 0;
}

/* Test PMIC get method */
static int dm_test_power_pmic_get(struct unit_test_state *uts)
{
	power_pmic_get(uts, "sandbox_pmic");

	return 0;
}
DM_TEST(dm_test_power_pmic_get, UT_TESTF_SCAN_FDT);

/* PMIC get method - MC34708 - for 3 bytes transmission */
static int dm_test_power_pmic_mc34708_get(struct unit_test_state *uts)
{
	power_pmic_get(uts, "pmic@41");

	return 0;
}

DM_TEST(dm_test_power_pmic_mc34708_get, UT_TESTF_SCAN_FDT);

/* Test PMIC I/O */
static int dm_test_power_pmic_io(struct unit_test_state *uts)
{
	const char *name = "sandbox_pmic";
	uint8_t out_buffer, in_buffer;
	struct udevice *dev;
	int reg_count, i;

	ut_assertok(pmic_get(name, &dev));

	reg_count = pmic_reg_count(dev);
	ut_asserteq(reg_count, SANDBOX_PMIC_REG_COUNT);

	/*
	 * Test PMIC I/O - write and read a loop counter.
	 * usually we can't write to all PMIC's registers in the real hardware,
	 * but we can to the sandbox pmic.
	 */
	for (i = 0; i < reg_count; i++) {
		out_buffer = i;
		ut_assertok(pmic_write(dev, i, &out_buffer, 1));
		ut_assertok(pmic_read(dev, i, &in_buffer, 1));
		ut_asserteq(out_buffer, in_buffer);
	}

	return 0;
}
DM_TEST(dm_test_power_pmic_io, UT_TESTF_SCAN_FDT);

#define MC34708_PMIC_REG_COUNT 64
#define MC34708_PMIC_TEST_VAL 0x125534
static int dm_test_power_pmic_mc34708_regs_check(struct unit_test_state *uts)
{
	struct udevice *dev;
	int reg_count;

	ut_assertok(pmic_get("pmic@41", &dev));

	/* Check number of PMIC registers */
	reg_count = pmic_reg_count(dev);
	ut_asserteq(reg_count, MC34708_PMIC_REG_COUNT);

	return 0;
}

DM_TEST(dm_test_power_pmic_mc34708_regs_check, UT_TESTF_SCAN_FDT);

static int dm_test_power_pmic_mc34708_rw_val(struct unit_test_state *uts)
{
	struct udevice *dev;
	int val;

	ut_assertok(pmic_get("pmic@41", &dev));

	/* Check if single 3 byte read is successful */
	val = pmic_reg_read(dev, REG_POWER_CTL2);
	ut_asserteq(val, 0x422100);

	/* Check if RW works */
	val = 0;
	ut_assertok(pmic_reg_write(dev, REG_RTC_TIME, val));
	ut_assertok(pmic_reg_write(dev, REG_RTC_TIME, MC34708_PMIC_TEST_VAL));
	val = pmic_reg_read(dev, REG_RTC_TIME);
	ut_asserteq(val, MC34708_PMIC_TEST_VAL);

	pmic_clrsetbits(dev, REG_POWER_CTL2, 0x3 << 8, 1 << 9);
	val = pmic_reg_read(dev, REG_POWER_CTL2);
	ut_asserteq(val, (0x422100 & ~(0x3 << 8)) | (1 << 9));

	return 0;
}

DM_TEST(dm_test_power_pmic_mc34708_rw_val, UT_TESTF_SCAN_FDT);
