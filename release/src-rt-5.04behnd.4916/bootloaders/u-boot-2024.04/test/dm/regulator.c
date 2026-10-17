// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for the driver model regulator API
 *
 * Copyright (c) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/sandbox_pmic.h>
#include <test/test.h>
#include <test/ut.h>

enum {
	BUCK1,
	BUCK2,
	BUCK3,
	LDO1,
	LDO2,
	OUTPUT_COUNT,
};

enum {
	DEVNAME = 0,
	PLATNAME,
	OUTPUT_NAME_COUNT,
};

static const char *regulator_names[OUTPUT_COUNT][OUTPUT_NAME_COUNT] = {
	/* devname, platname */
	{ SANDBOX_BUCK1_DEVNAME, SANDBOX_BUCK1_PLATNAME },
	{ SANDBOX_BUCK2_DEVNAME, SANDBOX_BUCK2_PLATNAME },
	{ SANDBOX_BUCK3_DEVNAME, SANDBOX_BUCK3_PLATNAME },
	{ SANDBOX_LDO1_DEVNAME, SANDBOX_LDO1_PLATNAME},
	{ SANDBOX_LDO2_DEVNAME, SANDBOX_LDO2_PLATNAME},
};

/* Test regulator get method */
static int dm_test_power_regulator_get(struct unit_test_state *uts)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *dev_by_devname;
	struct udevice *dev_by_platname;
	const char *devname;
	const char *platname;
	int i;

	for (i = 0; i < OUTPUT_COUNT; i++) {
		/*
		 * Do the test for each regulator's devname and platname,
		 * which are related to a single device.
		 */
		devname = regulator_names[i][DEVNAME];
		platname = regulator_names[i][PLATNAME];

		/*
		 * Check, that regulator_get_by_devname() function, returns
		 * a device with the name equal to the requested one.
		 */
		ut_assertok(regulator_get_by_devname(devname, &dev_by_devname));
		ut_asserteq_str(devname, dev_by_devname->name);

		/*
		 * Check, that regulator_get_by_platname() function, returns
		 * a device with the name equal to the requested one.
		 */
		ut_assertok(regulator_get_by_platname(platname, &dev_by_platname));
		uc_pdata = dev_get_uclass_plat(dev_by_platname);
		ut_assert(uc_pdata);
		ut_asserteq_str(platname, uc_pdata->name);

		/*
		 * Check, that the pointers returned by both get functions,
		 * points to the same regulator device.
		 */
		ut_asserteq_ptr(dev_by_devname, dev_by_platname);
	}

	return 0;
}
DM_TEST(dm_test_power_regulator_get, UT_TESTF_SCAN_FDT);

/* Test regulator set and get Voltage method */
static int dm_test_power_regulator_set_get_voltage(struct unit_test_state *uts)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *dev;
	const char *platname;
	int val_set, val_get;

	/* Set and get Voltage of BUCK1 - set to 'min' constraint */
	platname = regulator_names[BUCK1][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));

	uc_pdata = dev_get_uclass_plat(dev);
	ut_assert(uc_pdata);

	val_set = uc_pdata->min_uV;
	ut_assertok(regulator_set_value(dev, val_set));

	val_get = regulator_get_value(dev);
	ut_assert(val_get >= 0);

	ut_asserteq(val_set, val_get);

	return 0;
}
DM_TEST(dm_test_power_regulator_set_get_voltage, UT_TESTF_SCAN_FDT);

/* Test regulator set and get Current method */
static int dm_test_power_regulator_set_get_current(struct unit_test_state *uts)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *dev;
	const char *platname;
	int val_set, val_get;

	/* Set and get the Current of LDO1 - set to 'min' constraint */
	platname = regulator_names[LDO1][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));

	uc_pdata = dev_get_uclass_plat(dev);
	ut_assert(uc_pdata);

	val_set = uc_pdata->min_uA;
	ut_assertok(regulator_set_current(dev, val_set));

	val_get = regulator_get_current(dev);
	ut_assert(val_get >= 0);

	ut_asserteq(val_set, val_get);

	/* Check LDO2 current limit constraints - should be -ENODATA */
	platname = regulator_names[LDO2][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));

	uc_pdata = dev_get_uclass_plat(dev);
	ut_assert(uc_pdata);
	ut_asserteq(-ENODATA, uc_pdata->min_uA);
	ut_asserteq(-ENODATA, uc_pdata->max_uA);

	/* Try set the Current of LDO2 - should return -ENOSYS */
	ut_asserteq(-ENOSYS, regulator_set_current(dev, 0));

	return 0;
}
DM_TEST(dm_test_power_regulator_set_get_current, UT_TESTF_SCAN_FDT);

/* Test regulator set and get Enable method */
static int dm_test_power_regulator_set_get_enable(struct unit_test_state *uts)
{
	const char *platname;
	struct udevice *dev;
	bool val_set = true;

	/* Set the Enable of LDO1 - default is disabled */
	platname = regulator_names[LDO1][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));
	ut_assertok(regulator_set_enable(dev, val_set));

	/* Get the Enable state of LDO1 and compare it with the requested one */
	ut_asserteq(regulator_get_enable(dev), val_set);

	return 0;
}
DM_TEST(dm_test_power_regulator_set_get_enable, UT_TESTF_SCAN_FDT);

/* Test regulator set and get enable if allowed method */
static
int dm_test_power_regulator_set_enable_if_allowed(struct unit_test_state *uts)
{
	const char *platname;
	struct udevice *dev, *dev_autoset;
	bool val_set = false;

	/* Get BUCK1 - always on regulator */
	platname = regulator_names[BUCK1][PLATNAME];
	ut_assertok(regulator_autoset_by_name(platname, &dev_autoset));
	ut_assertok(regulator_get_by_platname(platname, &dev));

	/* Try disabling always-on regulator */
	ut_assertok(regulator_set_enable_if_allowed(dev, val_set));
	ut_asserteq(regulator_get_enable(dev), !val_set);

	return 0;
}
DM_TEST(dm_test_power_regulator_set_enable_if_allowed, UT_TESTF_SCAN_FDT);

/* Test regulator set and get mode method */
static int dm_test_power_regulator_set_get_mode(struct unit_test_state *uts)
{
	const char *platname;
	struct udevice *dev;
	int val_set = LDO_OM_SLEEP;

	/* Set the mode id to LDO_OM_SLEEP of LDO1 - default is LDO_OM_OFF */
	platname = regulator_names[LDO1][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));
	ut_assertok(regulator_set_mode(dev, val_set));

	/* Get the mode id of LDO1 and compare it with the requested one */
	ut_asserteq(regulator_get_mode(dev), val_set);

	return 0;
}
DM_TEST(dm_test_power_regulator_set_get_mode, UT_TESTF_SCAN_FDT);

/* Test regulator set and get suspend Voltage method */
static int dm_test_power_regulator_set_get_suspend_voltage(struct unit_test_state *uts)
{
	struct dm_regulator_uclass_plat *uc_pdata;
	const struct dm_regulator_ops *ops;
	struct udevice *dev;
	const char *platname;
	int val_set, val_get;

	/* Set and get Voltage of BUCK1 - set to 'min' constraint */
	platname = regulator_names[BUCK1][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));

	uc_pdata = dev_get_uclass_plat(dev);
	ut_assert(uc_pdata);

	ops = dev_get_driver_ops(dev);

	if (ops->set_suspend_value && ops->get_suspend_value) {
		val_set = uc_pdata->suspend_uV;
		ut_assertok(regulator_set_suspend_value(dev, val_set));
		val_get = regulator_get_suspend_value(dev);
		ut_assert(val_get >= 0);

		ut_asserteq(val_set, val_get);
	}
	return 0;
}
DM_TEST(dm_test_power_regulator_set_get_suspend_voltage, UT_TESTF_SCAN_FDT);

/* Test regulator set and get suspend Enable method */
static int dm_test_power_regulator_set_get_suspend_enable(struct unit_test_state *uts)
{
	const struct dm_regulator_ops *ops;
	const char *platname;
	struct udevice *dev;
	bool val_set = true;

	/* Set the Enable of LDO1 - default is disabled */
	platname = regulator_names[LDO1][PLATNAME];
	ut_assertok(regulator_get_by_platname(platname, &dev));

	ops = dev_get_driver_ops(dev);

	if (ops->set_suspend_enable && ops->get_suspend_enable) {
		ut_assertok(regulator_set_suspend_enable(dev, val_set));

		/*
		 * Get the Enable state of LDO1 and
		 * compare it with the requested one
		 */
		ut_asserteq(regulator_get_suspend_enable(dev), val_set);
	}
	return 0;
}
DM_TEST(dm_test_power_regulator_set_get_suspend_enable, UT_TESTF_SCAN_FDT);

/* Test regulator autoset method */
static int dm_test_power_regulator_autoset(struct unit_test_state *uts)
{
	const char *platname;
	struct udevice *dev, *dev_autoset;

	/*
	 * Test the BUCK1 with fdt properties
	 * - min-microvolt = max-microvolt = 1200000
	 * - min-microamp = max-microamp = 200000
	 * - always-on = set
	 * - boot-on = not set
	 * Expected output state: uV=1200000; uA=200000; output enabled
	 */
	platname = regulator_names[BUCK1][PLATNAME];
	ut_assertok(regulator_autoset_by_name(platname, &dev_autoset));

	/* Check, that the returned device is proper */
	ut_assertok(regulator_get_by_platname(platname, &dev));
	ut_asserteq_ptr(dev, dev_autoset);

	/* Check the setup after autoset */
	ut_asserteq(regulator_get_value(dev),
		    SANDBOX_BUCK1_AUTOSET_EXPECTED_UV);
	ut_asserteq(regulator_get_current(dev),
		    SANDBOX_BUCK1_AUTOSET_EXPECTED_UA);
	ut_asserteq(regulator_get_enable(dev),
		    SANDBOX_BUCK1_AUTOSET_EXPECTED_ENABLE);

	return 0;
}
DM_TEST(dm_test_power_regulator_autoset, UT_TESTF_SCAN_FDT);

/*
 * Struct setting: to keep the expected output settings.
 * @voltage: Voltage value [uV]
 * @current: Current value [uA]
 * @enable: output enable state: true/false
 */
struct setting {
	int voltage;
	int current;
	bool enable;
};

/*
 * platname_list: an array of regulator platform names.
 * For testing regulator_list_autoset() for outputs:
 * - LDO1
 * - LDO2
 */
static const char *platname_list[] = {
	SANDBOX_LDO1_PLATNAME,
	SANDBOX_LDO2_PLATNAME,
	NULL,
};

/*
 * expected_setting_list: an array of regulator output setting, expected after
 * call of the regulator_list_autoset() for the "platname_list" array.
 * For testing results of regulator_list_autoset() for outputs:
 * - LDO1
 * - LDO2
 * The settings are defined in: include/power/sandbox_pmic.h
 */
static const struct setting expected_setting_list[] = {
	[0] = { /* LDO1 */
	.voltage = SANDBOX_LDO1_AUTOSET_EXPECTED_UV,
	.current = SANDBOX_LDO1_AUTOSET_EXPECTED_UA,
	.enable  = SANDBOX_LDO1_AUTOSET_EXPECTED_ENABLE,
	},
	[1] = { /* LDO2 */
	.voltage = SANDBOX_LDO2_AUTOSET_EXPECTED_UV,
	.current = SANDBOX_LDO2_AUTOSET_EXPECTED_UA,
	.enable  = SANDBOX_LDO2_AUTOSET_EXPECTED_ENABLE,
	},
};

static int list_count = ARRAY_SIZE(expected_setting_list);

/* Test regulator list autoset method */
static int dm_test_power_regulator_autoset_list(struct unit_test_state *uts)
{
	struct udevice *dev_list[2], *dev;
	int i;

	/*
	 * Test the settings of the regulator list:
	 * LDO1 with fdt properties:
	 * - min-microvolt = max-microvolt = 1800000
	 * - min-microamp = max-microamp = 100000
	 * - always-on = not set
	 * - boot-on = set
	 * Expected output state: uV=1800000; uA=100000; output enabled
	 *
	 * LDO2 with fdt properties:
	 * - min-microvolt = max-microvolt = 3300000
	 * - always-on = not set
	 * - boot-on = not set
	 * Expected output state: uV=300000(default); output disabled(default)
	 * The expected settings are defined in: include/power/sandbox_pmic.h.
	 */
	ut_assertok(regulator_list_autoset(platname_list, dev_list, false));

	for (i = 0; i < list_count; i++) {
		/* Check, that the returned device is non-NULL */
		ut_assert(dev_list[i]);

		/* Check, that the returned device is proper */
		ut_assertok(regulator_get_by_platname(platname_list[i], &dev));
		ut_asserteq_ptr(dev_list[i], dev);

		/* Check, that regulator output Voltage value is as expected */
		ut_asserteq(regulator_get_value(dev_list[i]),
			    expected_setting_list[i].voltage);

		/* Check, that regulator output Current value is as expected */
		ut_asserteq(regulator_get_current(dev_list[i]),
			    expected_setting_list[i].current);

		/* Check, that regulator output Enable state is as expected */
		ut_asserteq(regulator_get_enable(dev_list[i]),
			    expected_setting_list[i].enable);
	}

	return 0;
}
DM_TEST(dm_test_power_regulator_autoset_list, UT_TESTF_SCAN_FDT);
